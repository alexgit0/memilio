#ifndef SECIR_H
#define SECIR_H

#include <epidemiology/damping.h>

#include <vector>
#include <Eigen/Core>

namespace epi
{

/**
 * Paramters of the SECIR/SECIHURD model:
 * T_inc (also sigma^(-1) or R_2^(-1)+R_3^(-1)): mean incubation period (default: 5.2);
 *          R_2^(-1) is the first part of the incubation time where the person is not yet infectioous
 *          R_3 is the exchange between asymptomatic carriers and infectious people; R_3^(-1) is the second part of the incubation time where the person is infectious WITHOUT showing symptoms
 * T_serint (also R_2^(-1)+0.5*R_3^(-1)): serial interval (default: 4.2);
 * T_infmild (also gamma^(-1) or R_4^(-1)): time a person remains infective after disease (if 'hospitalized' is considered a state, it does not apply to them but only to 'mildly infected' people in SECIR)
 * T_hosp2home (also R_5^(1)): duration for which the hospitalized patients not requiring further intensive care remain under general hospital care (=INF or R_5=0 in standard SEIR to waive influence of this parameter)
 * T_home2hosp (also R_6^(-1)): mean time a patient with mild symptoms spends at home before hospital admission due to worsening of the disease condition  (=INF or R_6=0 in standard SEIR to waive influence of this parameter)
 * T_hosp2icu (also R_7^(-1)): mean time a patient who entered the hospital will be hopistalized without ICU before being connected to an ICU  (=INF or R_7=0 in standard SEIR to waive influence of this parameter)
 * T_icu2home (also R_8^(-1)): mean time a patient is connected to an ICU before returning home (=INF or R_8=0 in standard SEIR to waive influence of this parameter)
 * T_infasy (also R_9^(-1)): mean time an asymptomatic person remains infective (=INF or R_9=0 in standard SEIR to waive influence of this parameter)
 * T_icu2death (also d; better would be R_10^(-1)): mean time a person needs ICU support before dying (=INF or R_10=0 in standard SEIR to waive influence of this parameter)
 * cont_freq (also R_1: contact frequency/rate; called beta in the standard SEIR model)
 * alpha: share of asymptomatic cases
 * beta (Not the beta in SEIR model): risk of infection from the infected symptomatic patients
 * rho: H/I; hospitalized per infected (=0 in standard SEIR)
 * theta: U/H; intensive care units per hospitalized
 * delta: D/U; deaths per intensive care units
**/
/**
     * @brief Initializes a SECIR/SECIHURD model
     *
     * @todo parameter description
     *
     * @param tinc
     * @param tinfmild
     * @param tserint
     * @param thosp2home
     * @param thome2hosp
     * @param thosp2icu
     * @param ticu2home
     * @param tinfasy
     * @param ticu2death
     * @param cont_freq_in
     * @param alpha_in
     * @param beta_in
     * @param delta_in
     * @param rho_in
     * @param theta_in
     * @param nb_total_t0_in
     * @param nb_exp_t0_in
     * @param nb_car_t0_in
     * @param nb_inf_t0_in
     * @param nb_hosp_t0_in
     * @param nb_icu_t0_in
     * @param nb_rec_t0_in
     * @param nb_dead_t0_in
     */
class SecirParams
{
public:
    double base_reprod;

    // time parameters for the different 'stages' of the disease of scale day or 1/day
    // 'stages' does not refer to the 'states' of the SECIR model but also includes incubation time or contact frequency
    class StageTimes
    {
    public:
        /**
         * @brief Initializes a time parameters' struct in the SECIR model
         */
        StageTimes();

        /**
         * @brief sets the incubation time in the SECIR model
         * @param tinc incubation time in day unit
         */
        void set_incubation(double const& tinc);

        /**
         * @brief sets the infectious time for symptomatic cases that are infected but who do not need to be hsopitalized in the SECIR model
         * @param tinfmild infectious time for symptomatic cases (if not hospitalized) in day unit 
         */
        void set_infectious_mild(double const& tinfmild);

        /**
         * @brief sets the serial interval in the SECIR model
         * @param tserint serial interval in day unit 
         */
        void set_serialinterval(double const& tserint);

        /**
         * @brief sets the time people are 'simply' hospitalized before returning home in the SECIR model
         * @param thosp2home time people are 'simply' hospitalized before returning home in day unit 
         */
        void set_hospitalized_to_home(double const& thosp2home);

        /**
         * @brief sets the time people are infectious at home before 'simply' hospitalized in the SECIR model
         * @param thome2hosp time people are infectious at home before 'simply' hospitalized in day unit 
         */
        void set_home_to_hospitalized(double const& thome2hosp);

        /**
         * @brief sets the time people are 'simply' hospitalized before being treated by ICU in the SECIR model
         * @param thosp2icu time people are 'simply' hospitalized before being treated by ICU in day unit 
         */
        void set_hospitalized_to_icu(double const& thosp2icu);

        /**
         * @brief sets the time people are treated by ICU before returning home in the SECIR model
         * @param ticu2home time people are treated by ICU before returning home in day unit 
         */
        void set_icu_to_home(double const& ticu2home);

        /**
         * @brief sets the infectious time for asymptomatic cases in the SECIR model
         * @param tinfasy infectious time for asymptomatic cases in day unit 
         */
        void set_infectious_asymp(double const& tinfasy);

        /**
         * @brief sets the time people are treated by ICU before dying in the SECIR model
         * @param ticu2death time people are treated by ICU before dying in day unit 
         */
        void set_icu_to_death(double const& ticu2death);

        /**
         * @brief returns 1.0 over the incubation time set for the SECIR model in day unit
         */
        double get_incubation_inv() const;

        /**
         * @brief returns 1.0 over the infectious time set for the SECIR model in day unit
         */
        double get_infectious_mild_inv() const;

        /**
         * @brief returns 1.0 over the serial interval in the SECIR model
         */
        double get_serialinterval_inv() const;

        /**
         * @brief returns 1.0 over the time people are 'simply' hospitalized before returning home in the SECIR model 
         */
        double get_hospitalized_to_home_inv() const;

        /**
         * @brief returns 1.0 over the time people are infectious at home before 'simply' hospitalized in the SECIR model 
         */
        double get_home_to_hospitalized_inv() const;

        /**
         * @brief returns 1.0 over the time people are 'simply' hospitalized before being treated by ICU in the SECIR model
         */
        double get_hospitalized_to_icu_inv() const;

        /**
         * @brief returns 1.0 over the time people are treated by ICU before returning home in the SECIR model
         */
        double get_icu_to_home_inv() const;

        /**
         * @brief returns 1.0 over the infectious time for asymptomatic cases in the SECIR model
         */
        double get_infectious_asymp_inv() const;

        /**
         * @brief returns 1.0 over the time people are treated by ICU before dying in the SECIR model
         */
        double get_icu_to_dead_inv() const;

    private:
        double m_tinc_inv, m_tinfmild_inv; // parameters also available in SEIR
        double m_tserint_inv, m_thosp2home_inv, m_thome2hosp_inv, m_thosp2icu_inv, m_ticu2home_inv, m_tinfasy_inv,
            m_ticu2death_inv; // new SECIR params
    };

    // population parameters of unit scale
    class Populations
    {
    public:
        /**
         * @brief Initializes a population parameters' struct in the SECIR model
         */
        Populations();

        /**
         * @brief sets the number of total people at t0 in the SECIR model
         * automatically calls set_suscetible_t0() to subtract from the total number
         * @param nb_total_t0 total number of people at t0
         */
        void set_total_t0(double nb_total_t0);

        /**
         * @brief sets the number of exposed people at t0 in the SECIR model
         * automatically calls set_suscetible_t0() to subtract from the total number
         * @param nb_exp_t0 number of exposed people at t0
         */
        void set_exposed_t0(double nb_exp_t0);

        /**
         * @brief sets the number of carrier people at t0 in the SECIR model
         * automatically calls set_suscetible_t0() to subtract from the total number
         * @param nb_car_t0 number of recovered people at t0
         */
        void set_carrier_t0(double nb_car_t0);

        /**
         * @brief sets the number of infectious people at t0 in the SECIR model
         * automatically calls set_suscetible_t0() to subtract from the total number
         * @param nb_inf_t0 number of infectious people at t0
         */
        void set_infectious_t0(double nb_inf_t0);

        /**
         * @brief sets the number of hospitalized people at t0 in the SECIR model
         * automatically calls set_suscetible_t0() to subtract from the total number
         * @param nb_hosp_t0 number of recovered people at t0
         */
        void set_hospital_t0(double nb_hosp_t0);

        /**
         * @brief sets the number of ICU-treated people at t0 in the SECIR model
         * automatically calls set_suscetible_t0() to subtract from the total number
         * @param nb_icu_t0 number of recovered people at t0
         */
        void set_icu_t0(double nb_icu_t0);

        /**
         * @brief sets the number of recovered people at t0 in the SECIR model
         * automatically calls set_suscetible_t0() to subtract from the total number
         * @param nb_rec_t0 number of recovered people at t0
         */
        void set_recovered_t0(double nb_rec_t0);

        /**
         * @brief sets the number of dead people at t0 in the SECIR model
         * automatically calls set_suscetible_t0() to subtract from the total number
         * @param nb_dead_t0 number of recovered people at t0
         */
        void set_dead_t0(double nb_dead_t0);

        /**
         * @brief sets the number of suscetible people at t0 in the SECIR model
         * only to be called after all other populations have been called
         */
        void set_suscetible_t0();

        /**
         * @brief returns the number of total people at t0 in the SECIR model
         */
        double get_total_t0() const;

        /**
         * @brief returns the number of exposed people at t0 in the SECIR model
         */
        double get_exposed_t0() const;

        /**
         * @brief returns the number of carrier people at t0 in the SECIR model
         */
        double get_carrier_t0() const;

        /**
         * @brief returns the number of infectious people at t0 in the SECIR model
         */
        double get_infectious_t0() const;

        /**
         * @brief returns the number of hospitalized people at t0 in the SECIR model
         */
        double get_hospitalized_t0() const;

        /**
         * @brief returns the number of ICU-treated people at t0 in the SECIR model
         */
        double get_icu_t0() const;

        /**
         * @brief returns the number of recovered people at t0 in the SECIR model
         */
        double get_recovered_t0() const;

        /**
         * @brief returns the number of dead people at t0 in the SECIR model
         */
        double get_dead_t0() const;

        /**
         * @brief returns the number of suscetible people at t0 in the SECIR model
         */
        double get_suscetible_t0() const;

    private:
        double m_nb_total_t0, m_nb_sus_t0, m_nb_exp_t0, m_nb_car_t0, m_nb_inf_t0, m_nb_hosp_t0, m_nb_icu_t0,
            m_nb_rec_t0, m_nb_dead_t0;
    };

    class Probabilities
    {
    public:
        /**
         * @brief Initializes a probabilites parameters' struct in the SECIR model
         */
        Probabilities();

        /**
        * @brief sets the percentage of asymptomatic cases in the SECIR model
        * @param alpha the percentage of asymptomatic cases
        */
        void set_asymp_per_infectious(double const& alpha);

        /**
        * @brief sets the risk of infection from symptomatic cases in the SECIR model
        * @param beta the risk of infection from symptomatic cases 
        */
        void set_risk_from_symptomatic(double const& beta);

        /**
        * @brief sets the percentage of hospitalized patients per infected patients in the SECIR model
        * @param rho percentage of hospitalized patients per infected patients
        */
        void set_hospitalized_per_infectious(double const& rho);

        /**
        * @brief sets the percentage of ICU patients per hospitalized patients in the SECIR model
        * @param theta percentage of ICU patients per hospitalized patients
        */
        void set_icu_per_hospitalized(double const& theta);

        /**
        * @brief sets the percentage of dead patients per ICU patients in the SECIR model
        * @param delta percentage of dead patients per ICU patients 
        */
        void set_dead_per_icu(double const& delta);

        /**
        * @brief returns the percentage of asymptomatic cases in the SECIR model
        */
        double get_asymp_per_infectious() const;

        /**
        * @brief returns the risk of infection from symptomatic cases in the SECIR model
        */
        double get_risk_from_symptomatic() const;

        /**
        * @brief returns the percentage of hospitalized patients per infected patients in the SECIR model
        */
        double get_hospitalized_per_infectious() const;

        /**
        * @brief returns the percentage of ICU patients per hospitalized patients in the SECIR model
        */
        double get_icu_per_hospitalized() const;

        /**
        * @brief returns the percentage of dead patients per ICU patients in the SECIR model
        */
        double get_dead_per_icu() const;

    private:
        double m_alpha, m_beta, m_rho, m_theta, m_delta; // probabilities
    };

    class ContactFrequencies
    {
    public:
        /**
         * @brief Initializes a contact frequencies parameters' struct in the SECIR model
         */
        ContactFrequencies();

        /**
         * @brief sets the contact frequency in the SECIR model
         * @param cont_freq contact rate/frequency in 1/day unit
         */
        void set_cont_freq(double const& cont_freq);

        /**
         * @brief returns the contact frequency set for the SECIR model in 1/day unit
         */
        double get_cont_freq() const;

    private:
        std::vector<std::vector<double>> m_cont_freq;
    };

    std::vector<StageTimes> times;

    std::vector<Populations> populations;

    std::vector<Probabilities> probabilities;

    ContactFrequencies contact_freq_matrix;

    // This defines a damping factor for a mitigation strategy for different points in time.
    std::vector<Dampings> dampings;

    /**
     * @brief Initializes a SECIR/SECIHURD model without default parameters 
     */
    SecirParams();
};

/**
 * @brief returns the actual, approximated reproduction rate 
 */
double get_reprod_rate(SecirParams const& params, double t, std::vector<double> const& yt);

/**
 * prints given parameters
 * @param[in] params the SecirParams parameter object
 */
void print_secir_params(SecirParams const& params);

/**
 * Computes the current time-derivative of S, E, C, I, (H, U,) R, (D) in the SECIR/SECIHURD model
 * @param[in] params SECIR/SECIHURD Model parameters, created by secir_param
 * @tparam T the datatype of the cases
 * @param[in] y current  S, E, C, I, (H, U,) R, (D) values at t; y: [0:S, 1:E, 2:I, 3:R]
 * @param[in] t time / current day
 * @param[out] dydt the values of the time derivatices of S, E, C, I, (H, U,) R, (D)
 */
void secir_get_derivatives(SecirParams const& params, Eigen::VectorXd const& y, double t, Eigen::VectorXd& dydt);

/**
 * Computes the SECIR curve by integration
 * @param[in] secir_0 Initial S, E, C, I, (H, U,) R, (D) values at t0
 * @param[in] t0 start time of simulation
 * @param[in] tmax end time of simulation
 * @param[in] dt initial time step
 * @param[in] params SECIR/SECIHURD model parameters
 *
 * @returns Vector of times t
 */
std::vector<double> simulate(double t0, double tmax, double dt, SecirParams const& params,
                             std::vector<Eigen::VectorXd>& secir);

} // namespace epi

#endif // SECIR_H
