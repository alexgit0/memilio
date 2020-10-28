#include <epidemiology_io/secir_parameters_io.h>
#include <epidemiology_io/secir_result_io.h>
#include <epidemiology_io/io.h>
#include <epidemiology/utils/memory.h>
#include <epidemiology/utils/uncertain_value.h>
#include <epidemiology/utils/stl_util.h>
#include <epidemiology/utils/graph.h>
#include <epidemiology/migration/migration.h>
#include <epidemiology/secir/damping.h>
#include <epidemiology/secir/populations.h>
#include <epidemiology/secir/uncertain_matrix.h>
#include <epidemiology/secir/secir.h>

#include <tixi.h>
#include <vector>
#include <iostream>
#include <string>
#include <random>
#include <boost/filesystem.hpp>

namespace epi
{

void write_element(const TixiDocumentHandle& handle, const std::string& path, const std::string& element_name,
                   const UncertainValue& element, int io_mode, int num_runs)
{
    auto element_path = path_join(path, element_name);

    if (io_mode == 0) {
        tixiAddDoubleElement(handle, path.c_str(), element_name.c_str(), (double)element, "%g");
    }
    else if (io_mode == 1 || io_mode == 2 || io_mode == 3) {
        auto distribution = element.get_distribution().get();
        write_distribution(handle, path, element_name, *distribution);
        if (io_mode == 2) {
            tixiAddDoubleElement(handle, element_path.c_str(), "Value", (double)element, "%g");
        }
    }
    else {
        // TODO error handling
        epi::log_error("Wrong input for io_mode.");
    }

    if (io_mode == 3) {
        std::vector<double> predef_sample;
        for (int run = 0; run < num_runs; run++) {
            predef_sample.push_back((double)element);
        }
        write_predef_sample(handle, element_path, predef_sample);
    }
}

void write_distribution(const TixiDocumentHandle& handle, const std::string& path, const std::string& element_name,
                        const ParameterDistribution& distribution)
{

    struct WriteDistVisitor : public ConstParameterDistributionVisitor {
        WriteDistVisitor(const std::string& xml_path, TixiDocumentHandle tixi_handle)
            : handle(tixi_handle)
            , element_path(xml_path)
        {
        }

        void visit(const ParameterDistributionNormal& normal_distribution) override
        {
            tixiAddTextElement(handle, element_path.c_str(), "Distribution", "Normal");
            tixiAddDoubleElement(handle, element_path.c_str(), "Mean", normal_distribution.get_mean(), "%g");
            tixiAddDoubleElement(handle, element_path.c_str(), "Deviation", normal_distribution.get_standard_dev(),
                                 "%g");
            tixiAddDoubleElement(handle, element_path.c_str(), "Min", normal_distribution.get_lower_bound(), "%g");
            tixiAddDoubleElement(handle, element_path.c_str(), "Max", normal_distribution.get_upper_bound(), "%g");
        }

        void visit(const ParameterDistributionUniform& uniform_distribution) override
        {
            tixiAddTextElement(handle, element_path.c_str(), "Distribution", "Uniform");
            tixiAddDoubleElement(handle, element_path.c_str(), "Min", uniform_distribution.get_lower_bound(), "%g");
            tixiAddDoubleElement(handle, element_path.c_str(), "Max", uniform_distribution.get_upper_bound(), "%g");
        }

        TixiDocumentHandle handle;
        std::string element_path;
    };

    tixiCreateElement(handle, path.c_str(), element_name.c_str());
    auto element_path = path_join(path, element_name);

    WriteDistVisitor visitor(element_path, handle);
    distribution.accept(visitor);

    tixiAddFloatVector(handle, element_path.c_str(), "PredefinedSamples", distribution.get_predefined_samples().data(),
                       static_cast<int>(distribution.get_predefined_samples().size()), "%g");
}

std::unique_ptr<UncertainValue> read_element(TixiDocumentHandle handle, const std::string& path, int io_mode)
{
    std::unique_ptr<UncertainValue> value;
    ReturnCode status;

    if (io_mode == 0) {
        double read_buffer;
        status = tixiGetDoubleElement(handle, path.c_str(), &read_buffer);
        assert(status == SUCCESS && ("Failed to read value at " + path).c_str());
        value = std::make_unique<UncertainValue>(read_buffer);
    }
    else if (io_mode == 1 || io_mode == 2 || io_mode == 3) {
        std::unique_ptr<ParameterDistribution> distribution = read_distribution(handle, path);

        if (io_mode == 2) {
            double read_buffer;
            status = tixiGetDoubleElement(handle, path_join(path, "Value").c_str(), &read_buffer);
            assert(status == SUCCESS && ("Failed to read value at " + path).c_str());
            value = std::make_unique<UncertainValue>(read_buffer);
        }
        value->set_distribution(*distribution.get());
    }
    else {
        // TODO error handling
        epi::log_error("Wrong input for io_mode.");
    }
    return value;
}

std::unique_ptr<ParameterDistribution> read_distribution(TixiDocumentHandle handle, const std::string& path)
{
    ReturnCode status;
    std::unique_ptr<ParameterDistribution> distribution;

    char* distri_str;
    status = tixiGetTextElement(handle, path_join(path, "Distribution").c_str(), &distri_str);
    assert(status == SUCCESS && ("Failed to read distribution type at " + path).c_str());
    if (strcmp("Normal", distri_str) == 0) {
        double mean;
        double dev;
        double min;
        double max;
        status = tixiGetDoubleElement(handle, path_join(path, "Mean").c_str(), &mean);
        assert(status == SUCCESS && ("Failed to read mean at " + path).c_str());

        status = tixiGetDoubleElement(handle, path_join(path, "Deviation").c_str(), &dev);
        assert(status == SUCCESS && ("Failed to read deviation at " + path).c_str());

        status = tixiGetDoubleElement(handle, path_join(path, "Min").c_str(), &min);
        assert(status == SUCCESS && ("Failed to read min value at " + path).c_str());

        status = tixiGetDoubleElement(handle, path_join(path, "Max").c_str(), &max);
        assert(status == SUCCESS && ("Failed to read max value at " + path).c_str());

        distribution = std::make_unique<ParameterDistributionNormal>(min, max, mean, dev);
    }
    else if (strcmp("Uniform", distri_str) == 0) {
        double min;
        double max;
        status = tixiGetDoubleElement(handle, path_join(path, "Min").c_str(), &min);
        assert(status == SUCCESS && ("Failed to read min value at " + path).c_str());

        status = tixiGetDoubleElement(handle, path_join(path, "Max").c_str(), &max);
        assert(status == SUCCESS && ("Failed to read max value at " + path).c_str());

        distribution = std::make_unique<ParameterDistributionUniform>(min, max);
    }
    else {
        // TODO: true error handling
        epi::log_error("Unknown distribution.");
        assert(false && "Unknown distribution.");
    }

    auto predef_path = path_join(path, "PredefinedSamples");
    int n_predef;
    tixiGetVectorSize(handle, predef_path.c_str(), &n_predef);

    double* predef = nullptr;
    tixiGetFloatVector(handle, predef_path.c_str(), &predef, n_predef);

    for (int i = 0; i < n_predef; i++) {
        distribution->add_predefined_sample(predef[i]);
    }

    return distribution;
}

void write_predef_sample(TixiDocumentHandle handle, const std::string& path, const std::vector<double>& samples)
{
    tixiRemoveElement(handle, path_join(path, "PredefinedSamples").c_str());
    tixiAddFloatVector(handle, path.c_str(), "PredefinedSamples", samples.data(), static_cast<int>(samples.size()),
                       "%g");
}

void write_contact(TixiDocumentHandle handle, const std::string& path, const UncertainContactMatrix& contact_pattern,
                   int io_mode)
{
    ContactFrequencyMatrix const& contact_freq_matrix = contact_pattern.get_cont_freq_mat();
    int num_groups                                    = contact_freq_matrix.get_size();
    tixiCreateElement(handle, path.c_str(), "ContactFreq");
    auto contact_path = path_join(path, "ContactFreq");
    for (int i = 0; i < num_groups; i++) {
        std::vector<double> row = {};
        for (int j = 0; j < num_groups; j++) {
            row.emplace_back(contact_freq_matrix.get_cont_freq(i, j));
        }
        tixiAddFloatVector(handle, contact_path.c_str(), ("ContactRateGroup_" + std::to_string(i + 1)).c_str(),
                           row.data(), num_groups, "%g");
    }
    for (int i = 0; i < num_groups; i++) {
        for (int j = 0; j < num_groups; j++) {
            int num_damp = static_cast<int>(contact_freq_matrix.get_dampings(i, j).get_dampings_vector().size());
            std::vector<double> row = {};
            for (int k = 0; k < num_damp; k++) {
                row.emplace_back(contact_freq_matrix.get_dampings(i, j).get_dampings_vector()[k].day);
                row.emplace_back(contact_freq_matrix.get_dampings(i, j).get_dampings_vector()[k].factor);
            }
            tixiAddFloatVector(handle, contact_path.c_str(),
                               ("DampingsGroups_" + std::to_string(i + 1) + "_" + std::to_string(j + 1)).c_str(),
                               row.data(), 2 * num_damp, "%g");
        }
    }

    if (io_mode == 1 || io_mode == 2 || io_mode == 3) {
        write_distribution(handle, contact_path, "NumDampings", *contact_pattern.get_distribution_damp_nb().get());
        write_distribution(handle, contact_path, "DampingDay", *contact_pattern.get_distribution_damp_days().get());
        write_distribution(handle, contact_path, "DampingDiagBase",
                           *contact_pattern.get_distribution_damp_diag_base().get());
        write_distribution(handle, contact_path, "DampingDiagRel",
                           *contact_pattern.get_distribution_damp_diag_rel().get());
        write_distribution(handle, contact_path, "DampingOffdiagRel",
                           *contact_pattern.get_distribution_damp_offdiag_rel().get());
    }
}

UncertainContactMatrix read_contact(TixiDocumentHandle handle, const std::string& path, int io_mode)
{

    ReturnCode status;
    int num_groups;
    status = tixiGetIntegerElement(handle, path_join("/Parameters", "NumberOfGroups").c_str(), &num_groups);
    assert(status == SUCCESS && ("Failed to read num_groups at " + path).c_str());

    UncertainContactMatrix contact_patterns{ContactFrequencyMatrix{(size_t)num_groups}};
    for (int i = 0; i < num_groups; i++) {
        double* row = nullptr;
        status = tixiGetFloatVector(handle, path_join(path, "ContactRateGroup_" + std::to_string(i + 1)).c_str(), &row,
                                    num_groups);
        assert(status == SUCCESS && ("Failed to read contact rate at " + path).c_str());

        for (int j = 0; j < num_groups; ++j) {
            contact_patterns.get_cont_freq_mat().set_cont_freq(row[j], i, j);
        }
    }

    for (int i = 0; i < num_groups; i++) {
        for (int j = 0; j < num_groups; j++) {
            int num_dampings;
            status = tixiGetVectorSize(
                handle,
                path_join(path, ("DampingsGroups_" + std::to_string(i + 1) + "_" + std::to_string(j + 1))).c_str(),
                &num_dampings);
            assert(status == SUCCESS && ("Failed to read num_dampings at " + path).c_str());

            double* dampings = nullptr;
            status           = tixiGetFloatVector(
                handle,
                path_join(path, ("DampingsGroups_" + std::to_string(i + 1) + "_" + std::to_string(j + 1))).c_str(),
                &dampings, num_dampings);
            assert(status == SUCCESS && ("Failed to read dampings at " + path).c_str());

            for (int k = 0; k < num_dampings / 2; k++) {
                contact_patterns.get_cont_freq_mat().add_damping(Damping{dampings[2 * k], dampings[2 * k + 1]}, i, j);
            }
        }
    }

    if (io_mode == 1 || io_mode == 2 || io_mode == 3) {
        contact_patterns.set_distribution_damp_nb(*read_distribution(handle, path_join(path, "NumDampings")));
        contact_patterns.set_distribution_damp_days(*read_distribution(handle, path_join(path, "DampingDay")));
        contact_patterns.set_distribution_damp_diag_base(
            *read_distribution(handle, path_join(path, "DampingDiagBase")));
        contact_patterns.set_distribution_damp_diag_rel(*read_distribution(handle, path_join(path, "DampingDiagRel")));
        contact_patterns.set_distribution_damp_offdiag_rel(
            *read_distribution(handle, path_join(path, "DampingOffdiagRel")));
    }
    return contact_patterns;
}

ParameterStudy read_parameter_study(TixiDocumentHandle handle, const std::string& path)
{
    ReturnCode status;

    int io_mode;
    int num_runs;
    double t0;
    double tmax;

    status = tixiGetIntegerElement(handle, path_join(path, "IOMode").c_str(), &io_mode);
    assert(status == SUCCESS && ("Failed to read io_mode at " + path).c_str());

    status = tixiGetIntegerElement(handle, path_join(path, "Runs").c_str(), &num_runs);
    assert(status == SUCCESS && ("Failed to read num_runs at " + path).c_str());

    status = tixiGetDoubleElement(handle, path_join(path, "T0").c_str(), &t0);
    assert(status == SUCCESS && ("Failed to read t0 at " + path).c_str());

    status = tixiGetDoubleElement(handle, path_join(path, "TMax").c_str(), &tmax);
    assert(status == SUCCESS && ("Failed to read tmax at " + path).c_str());

    return ParameterStudy(read_parameter_space(handle, path, io_mode), t0, tmax, num_runs);
}

SecirParams read_parameter_space(TixiDocumentHandle handle, const std::string& path, int io_mode)
{
    ReturnCode status;

    int num_groups;
    status = tixiGetIntegerElement(handle, path_join(path, "NumberOfGroups").c_str(), &num_groups);
    assert(status == SUCCESS && ("Failed to read num_groups at " + path).c_str());

    SecirParams params{(size_t)num_groups};

    double read_buffer;
    status = tixiGetDoubleElement(handle, path_join(path, "StartDay").c_str(), &read_buffer);
    assert(status == SUCCESS && ("Failed to read StartDay at " + path).c_str());

    params.set_start_day(read_buffer);
    params.set_seasonality(*read_element(handle, path_join(path, "Seasonality"), io_mode));
    params.set_icu_capacity(*read_element(handle, path_join(path, "ICUCapacity"), io_mode));

    params.set_contact_patterns(read_contact(handle, path_join(path, "ContactFreq"), io_mode));

    for (size_t i = 0; i < static_cast<size_t>(num_groups); i++) {
        auto group_name = "Group" + std::to_string(i + 1);
        auto group_path = path_join(path, group_name);

        // populations
        auto population_path = path_join(group_path, "Population");

        status = tixiGetDoubleElement(handle, path_join(population_path, "Dead").c_str(), &read_buffer);
        assert(status == SUCCESS && ("Failed to read number of deaths at " + path).c_str());

        params.populations.set({i, SecirCompartments::D}, read_buffer);

        params.populations.set({i, SecirCompartments::E},
                               *read_element(handle, path_join(population_path, "Exposed"), io_mode));
        params.populations.set({i, SecirCompartments::C},
                               *read_element(handle, path_join(population_path, "Carrier"), io_mode));
        params.populations.set({i, SecirCompartments::I},
                               *read_element(handle, path_join(population_path, "Infectious"), io_mode));
        params.populations.set({i, SecirCompartments::H},
                               *read_element(handle, path_join(population_path, "Hospitalized"), io_mode));
        params.populations.set({i, SecirCompartments::U},
                               *read_element(handle, path_join(population_path, "ICU"), io_mode));
        params.populations.set({i, SecirCompartments::R},
                               *read_element(handle, path_join(population_path, "Recovered"), io_mode));

        status = tixiGetDoubleElement(handle, path_join(population_path, "Total").c_str(), &read_buffer);
        assert(status == SUCCESS && ("Failed to read total population at " + path).c_str());

        params.populations.set_difference_from_group_total({i, SecirCompartments::S}, epi::SecirCategory::AgeGroup, i,
                                                           read_buffer);

        // times
        auto times_path = path_join(group_path, "StageTimes");

        params.times[i].set_incubation(*read_element(handle, path_join(times_path, "Incubation"), io_mode));
        params.times[i].set_infectious_mild(*read_element(handle, path_join(times_path, "InfectiousMild"), io_mode));
        params.times[i].set_serialinterval(*read_element(handle, path_join(times_path, "SerialInterval"), io_mode));
        params.times[i].set_hospitalized_to_home(
            *read_element(handle, path_join(times_path, "HospitalizedToRecovered"), io_mode));
        params.times[i].set_home_to_hospitalized(
            *read_element(handle, path_join(times_path, "InfectiousToHospitalized"), io_mode));
        params.times[i].set_infectious_asymp(*read_element(handle, path_join(times_path, "InfectiousAsympt"), io_mode));
        params.times[i].set_hospitalized_to_icu(
            *read_element(handle, path_join(times_path, "HospitalizedToICU"), io_mode));
        params.times[i].set_icu_to_home(*read_element(handle, path_join(times_path, "ICUToRecovered"), io_mode));
        params.times[i].set_icu_to_death(*read_element(handle, path_join(times_path, "ICUToDead"), io_mode));

        // probabilities
        auto probabilities_path = path_join(group_path, "Probabilities");

        params.probabilities[i].set_infection_from_contact(
            *read_element(handle, path_join(probabilities_path, "InfectedFromContact"), io_mode));
        params.probabilities[i].set_carrier_infectability(
            *read_element(handle, path_join(probabilities_path, "Carrierinfectability"), io_mode));
        params.probabilities[i].set_asymp_per_infectious(
            *read_element(handle, path_join(probabilities_path, "AsympPerInfectious"), io_mode));
        params.probabilities[i].set_risk_from_symptomatic(
            *read_element(handle, path_join(probabilities_path, "RiskFromSymptomatic"), io_mode));
        params.probabilities[i].set_dead_per_icu(
            *read_element(handle, path_join(probabilities_path, "DeadPerICU"), io_mode));
        params.probabilities[i].set_hospitalized_per_infectious(
            *read_element(handle, path_join(probabilities_path, "HospitalizedPerInfectious"), io_mode));
        params.probabilities[i].set_icu_per_hospitalized(
            *read_element(handle, path_join(probabilities_path, "ICUPerHospitalized"), io_mode));
    }

    return params;
}

void write_parameter_space(TixiDocumentHandle handle, const std::string& path, const SecirParams& parameters,
                           int num_runs, int io_mode)
{
    auto num_groups = parameters.get_num_groups();
    tixiAddIntegerElement(handle, path.c_str(), "NumberOfGroups", static_cast<int>(num_groups), "%d");

    tixiAddDoubleElement(handle, path.c_str(), "StartDay", parameters.get_start_day(), "%g");
    write_element(handle, path, "Seasonality", parameters.get_seasonality(), io_mode, num_runs);
    write_element(handle, path, "ICUCapacity", parameters.get_icu_capacity(), io_mode, num_runs);

    for (size_t i = 0; i < num_groups; i++) {
        auto group_name = "Group" + std::to_string(i + 1);
        auto group_path = path_join(path, group_name);

        tixiCreateElement(handle, path.c_str(), group_name.c_str());

        // populations
        auto population_path = path_join(group_path, "Population");
        tixiCreateElement(handle, group_path.c_str(), "Population");

        tixiAddDoubleElement(handle, population_path.c_str(), "Total",
                             parameters.populations.get_group_total(AgeGroup, i), "%g");
        tixiAddDoubleElement(handle, population_path.c_str(), "Dead",
                             parameters.populations.get({i, SecirCompartments::D}), "%g");
        write_element(handle, population_path, "Exposed", parameters.populations.get({i, SecirCompartments::E}),
                      io_mode, num_runs);
        write_element(handle, population_path, "Carrier", parameters.populations.get({i, SecirCompartments::C}),
                      io_mode, num_runs);
        write_element(handle, population_path, "Infectious", parameters.populations.get({i, SecirCompartments::I}),
                      io_mode, num_runs);
        write_element(handle, population_path, "Hospitalized", parameters.populations.get({i, SecirCompartments::H}),
                      io_mode, num_runs);
        write_element(handle, population_path, "ICU", parameters.populations.get({i, SecirCompartments::U}), io_mode,
                      num_runs);
        write_element(handle, population_path, "Recovered", parameters.populations.get({i, SecirCompartments::R}),
                      io_mode, num_runs);

        // times
        auto times_path = path_join(group_path, "StageTimes");
        tixiCreateElement(handle, group_path.c_str(), "StageTimes");

        write_element(handle, times_path, "Incubation", parameters.times[i].get_incubation(), io_mode, num_runs);
        write_element(handle, times_path, "InfectiousMild", parameters.times[i].get_infectious_mild(), io_mode,
                      num_runs);
        write_element(handle, times_path, "SerialInterval", parameters.times[i].get_serialinterval(), io_mode,
                      num_runs);
        write_element(handle, times_path, "HospitalizedToRecovered", parameters.times[i].get_hospitalized_to_home(),
                      io_mode, num_runs);
        write_element(handle, times_path, "InfectiousToHospitalized", parameters.times[i].get_home_to_hospitalized(),
                      io_mode, num_runs);
        write_element(handle, times_path, "InfectiousAsympt", parameters.times[i].get_infectious_asymp(), io_mode,
                      num_runs);
        write_element(handle, times_path, "HospitalizedToICU", parameters.times[i].get_hospitalized_to_icu(), io_mode,
                      num_runs);
        write_element(handle, times_path, "ICUToRecovered", parameters.times[i].get_icu_to_home(), io_mode, num_runs);
        write_element(handle, times_path, "ICUToDead", parameters.times[i].get_icu_to_dead(), io_mode, num_runs);

        // probabilities
        auto probabilities_path = path_join(group_path, "Probabilities");
        tixiCreateElement(handle, group_path.c_str(), "Probabilities");

        write_element(handle, probabilities_path, "InfectedFromContact",
                      parameters.probabilities[i].get_infection_from_contact(), io_mode, num_runs);
        write_element(handle, probabilities_path, "Carrierinfectability",
                      parameters.probabilities[i].get_carrier_infectability(), io_mode, num_runs);
        write_element(handle, probabilities_path, "AsympPerInfectious",
                      parameters.probabilities[i].get_asymp_per_infectious(), io_mode, num_runs);
        write_element(handle, probabilities_path, "RiskFromSymptomatic",
                      parameters.probabilities[i].get_risk_from_symptomatic(), io_mode, num_runs);
        write_element(handle, probabilities_path, "DeadPerICU", parameters.probabilities[i].get_dead_per_icu(), io_mode,
                      num_runs);
        write_element(handle, probabilities_path, "HospitalizedPerInfectious",
                      parameters.probabilities[i].get_hospitalized_per_infectious(), io_mode, num_runs);
        write_element(handle, probabilities_path, "ICUPerHospitalized",
                      parameters.probabilities[i].get_icu_per_hospitalized(), io_mode, num_runs);
    }

    write_contact(handle, path, parameters.get_contact_patterns(), io_mode);
}

void write_parameter_study(TixiDocumentHandle handle, const std::string& path, const ParameterStudy& parameter_study,
                           int io_mode)
{
    tixiAddIntegerElement(handle, path.c_str(), "IOMode", io_mode, "%d");
    tixiAddIntegerElement(handle, path.c_str(), "Runs", parameter_study.get_num_runs(), "%d");
    tixiAddDoubleElement(handle, path.c_str(), "T0", parameter_study.get_t0(), "%g");
    tixiAddDoubleElement(handle, path.c_str(), "TMax", parameter_study.get_tmax(), "%g");

    write_parameter_space(handle, path, parameter_study.get_secir_params(), parameter_study.get_num_runs(), io_mode);
}

void write_single_run_params(const int run, epi::Graph<epi::ModelNode<epi::SecirSimulation>, epi::MigrationEdge> graph,
                             double t0, double tmax)
{

    boost::filesystem::path dir("results");

    bool created = boost::filesystem::create_directory(dir);

    if (created) {
        log_info("Directory '{:s}' was created. Results are stored in {:s}/results.", dir.string(),
                 epi::get_current_dir_name());
    }
    else {
        log_info(
            "Directory '{:s}' already exists. Results are stored in {:s}/ results. Files from previous runs will be "
            "overwritten",
            dir.string(), epi::get_current_dir_name());
    }
    int node_id = 0;
    for (auto& node : graph.nodes()) {
        int num_runs     = 1;
        std::string path = "/Parameters";
        TixiDocumentHandle handle;
        tixiCreateDocument("Parameters", &handle);
        ParameterStudy study(node.get_params(), t0, tmax, num_runs);

        write_parameter_study(handle, path, study);

        tixiSaveDocument(handle,
                         (dir / ("Parameters_run" + std::to_string(run) + "_node" + std::to_string(node_id) + ".xml"))
                             .string()
                             .c_str());
        tixiCloseDocument(handle);

        save_result(node.get_result(),
                    (dir / ("Results_run" + std::to_string(run) + "_node" + std::to_string(node_id) + ".h5")).string());
        node_id++;
    }
}

void write_node(TixiDocumentHandle handle, const Graph<SecirParams, MigrationEdge>& graph, int node)
{
    int num_runs = 1;
    int io_mode  = 2;

    std::string path = "/Parameters";

    tixiAddIntegerElement(handle, path.c_str(), "NodeID", node, "%d");

    auto params = graph.nodes()[node];

    write_parameter_space(handle, path, params, num_runs, io_mode);
}

void read_node(TixiDocumentHandle node_handle, Graph<SecirParams, MigrationEdge>& graph)
{

    graph.add_node(read_parameter_space(node_handle, "/Parameters", 2));
}

void write_edge(const std::vector<TixiDocumentHandle>& edge_handles, const std::string& path,
                const Graph<SecirParams, MigrationEdge>& graph, int edge)
{

    int num_groups  = static_cast<int>(graph.nodes()[0].get_num_groups());
    int num_compart = static_cast<int>(graph.nodes()[0].populations.get_num_compartments()) / num_groups;

    auto start_node = static_cast<int>(graph.edges()[edge].start_node_idx);
    auto end_node   = static_cast<int>(graph.edges()[edge].end_node_idx);
    auto handle     = edge_handles[start_node];

    std::string edge_path = path_join(path, "EdgeTo" + std::to_string(end_node));
    tixiCreateElement(handle, path.c_str(), ("EdgeTo" + std::to_string(end_node)).c_str());
    tixiAddIntegerElement(handle, edge_path.c_str(), "StartNode", static_cast<int>(graph.edges()[edge].start_node_idx),
                          "%d");
    tixiAddIntegerElement(handle, edge_path.c_str(), "EndNode", static_cast<int>(graph.edges()[edge].end_node_idx),
                          "%d");
    for (int group = 0; group < num_groups; group++) {
        std::vector<double> weights;
        for (int compart = 0; compart < num_compart; compart++) {
            weights.push_back(graph.edges()[edge].property.coefficients[compart + group * num_compart]);
        }
        tixiAddFloatVector(handle, edge_path.c_str(), ("Group" + std::to_string(group + 1)).c_str(), weights.data(),
                           num_compart, "%g");
    }
}

void read_edge(const std::vector<TixiDocumentHandle>& edge_handles, const std::string& path,
               Graph<SecirParams, MigrationEdge>& graph, int start_node, int end_node)
{
    ReturnCode status;

    auto handle           = edge_handles[start_node];
    std::string edge_path = path_join(path, "EdgeTo" + std::to_string(end_node));
    int num_groups;
    int num_compart;

    status = tixiGetIntegerElement(handle, path_join(path, "NumberOfGroups").c_str(), &num_groups);
    assert(status == SUCCESS && ("Failed to read num_groups at " + path).c_str());

    status = tixiGetIntegerElement(handle, path_join(path, "NumberOfCompartiments").c_str(), &num_compart);
    assert(status == SUCCESS && ("Failed to read num_compart at " + path).c_str());

    auto all_weights = Eigen::VectorXd(num_compart * num_groups);
    for (int group = 0; group < num_groups; group++) {
        double* weights = nullptr;
        status = tixiGetFloatVector(handle, path_join(edge_path, "Group" + std::to_string(group + 1)).c_str(), &weights,
                                    num_compart);
        if (status == SUCCESS) {
            for (int compart = 0; compart < num_compart; compart++) {
                all_weights(compart + group * num_compart) = weights[compart];
            }
            graph.add_edge(start_node, end_node, all_weights);
        }
    }
}

void write_graph(const Graph<SecirParams, MigrationEdge>& graph, const std::string& dir_string)
{

    boost::filesystem::path dir(dir_string);

    bool created = boost::filesystem::create_directory(dir);

    if (created) {
        log_info("Directory '{:s}' was created. Results are stored in {:s}.", dir.string(),
                 path_join(epi::get_current_dir_name(), dir.string()));
    }
    else {
        log_info("Directory '{:s}' already exists. Results are stored in {:s}. Files from previous "
                 "graph will be "
                 "overwritten",
                 dir.string(), path_join(epi::get_current_dir_name(), dir.string()));
    }

    int num_nodes   = static_cast<int>(graph.nodes().size());
    int num_edges   = static_cast<int>(graph.edges().size());
    int num_groups  = graph.nodes()[0].get_contact_patterns().get_cont_freq_mat().get_size();
    int num_compart = static_cast<int>(graph.nodes()[0].populations.get_num_compartments()) / num_groups;

    std::vector<TixiDocumentHandle> edge_handles(num_nodes);
    std::string edges_path = "/Edges";
    for (auto& current_handle : edge_handles) {
        tixiCreateDocument("Edges", &current_handle);

        tixiAddIntegerElement(current_handle, edges_path.c_str(), "NumberOfNodes", num_nodes, "%d");
        tixiAddIntegerElement(current_handle, edges_path.c_str(), "NumberOfEdges", num_edges, "%d");
        tixiAddIntegerElement(current_handle, edges_path.c_str(), "NumberOfGroups", num_groups, "%d");
        tixiAddIntegerElement(current_handle, edges_path.c_str(), "NumberOfCompartiments", num_compart, "%d");
    }

    for (int edge = 0; edge < num_edges; edge++) {
        write_edge(edge_handles, edges_path, graph, edge);
    }

    for (int node = 0; node < num_nodes; node++) {
        tixiSaveDocument(edge_handles[node],
                         (dir / ("GraphEdges_node" + std::to_string(node) + ".xml")).string().c_str());
        tixiCloseDocument(edge_handles[node]);
    }

    for (int node = 0; node < num_nodes; node++) {
        TixiDocumentHandle node_handle;
        tixiCreateDocument("Parameters", &node_handle);
        write_node(node_handle, graph, node);
        tixiSaveDocument(node_handle, (dir / ("GraphNode" + std::to_string(node) + ".xml")).string().c_str());
        tixiCloseDocument(node_handle);
    }
}

Graph<SecirParams, MigrationEdge> read_graph(const std::string& dir_string)
{
    boost::filesystem::path dir(dir_string);
    auto check_dir = boost::filesystem::exists(dir);
    assert(check_dir && ("Directory " + dir_string + " does not exist.").c_str());

    ReturnCode status;
    TixiDocumentHandle handle;
    tixiOpenDocument((dir / "GraphEdges_node0.xml").string().c_str(), &handle);

    std::string edges_path = "/Edges";

    int num_nodes;
    int num_edges;

    status = tixiGetIntegerElement(handle, path_join(edges_path, "NumberOfNodes").c_str(), &num_nodes);
    assert(status == SUCCESS && ("Failed to read num_nodes at " + edges_path).c_str());

    status = tixiGetIntegerElement(handle, path_join(edges_path, "NumberOfEdges").c_str(), &num_edges);
    assert(status == SUCCESS && ("Failed to read num_edges at " + edges_path).c_str());

    std::vector<TixiDocumentHandle> edge_handles(num_nodes);

    Graph<SecirParams, MigrationEdge> graph;

    for (int node = 0; node < num_nodes; node++) {
        TixiDocumentHandle node_handle;
        tixiOpenDocument((dir / ("GraphNode" + std::to_string(node) + ".xml")).string().c_str(), &node_handle);
        read_node(node_handle, graph);
        tixiCloseDocument(node_handle);
    }

    for (int start_node = 0; start_node < num_nodes; start_node++) {
        tixiOpenDocument((dir / ("GraphEdges_node" + std::to_string(start_node) + ".xml")).string().c_str(),
                         &edge_handles[start_node]);
        for (int end_node = 0; end_node < num_nodes; end_node++) {
            read_edge(edge_handles, edges_path, graph, start_node, end_node);
        }

        tixiCloseDocument(edge_handles[start_node]);
    }
    return graph;
}

} // namespace epi
