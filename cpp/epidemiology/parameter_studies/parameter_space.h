#ifndef PARAMETER_SPACE_H
#define PARAMETER_SPACE_H

#include <vector>
#include <string>

namespace epi
{

/* TODO: Add more distributions here. */
typedef enum
{
    DIST_UNIFORM
} parameter_distribution;

struct parameter_info
{
    std::string name; /*< The name of this parameter */
    double min_value; /*< The minumum value of this parameter */
    double max_value; /*< The maximum value of this parameter */
    parameter_distribution dist; /*< The statistical distribution of this parameter */
};

/* The class parameter_space_t stores ranges of parameters
 * together with information on step sizes,
 * a start and end time as well as an initial time step.
 * The class provides an iterator that iterates over all 
 * generated parameter combinations.
 * 
 * Currently all parameters are of type double.
 */
class parameter_space_t
{
public:
    /* Constructor
     * \param [in] paramter_filename filename of a file storing ranges of input parameters.
     * Reads parameter names and values from an input file.
     */
    parameter_space_t (std::string &parameter_filename);

private:
    // The names of all stored parameters
    std::vector<std::string> parameter_names;
    
    // The start values of the parameters
    std::vector<double> parameter_start_values;
    // The end values of the parameters
    std::vector<double> parameter_end_values;
    // The step size values of the parameters
    std::vector<double> parameter_step_values;

    // Start time (should be the same for all simulations)
    T t0;
    // End time (should be the same for all simulations)
    T tmax;
    // time step (should be the same for all simulations)
    T dt;
}


#endif // PARAMETER_SPACE_H