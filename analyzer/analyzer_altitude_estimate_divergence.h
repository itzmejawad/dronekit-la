#ifndef ANALYZER_ALTITUDE_ESTIMATE_DIVERGENCE_H
#define ANALYZER_ALTITUDE_ESTIMATE_DIVERGENCE_H

/*
 * analyzer_altitude_estimate_divergence
 *
 */

#include "analyzer_estimate_divergence.h"

class Analyzer_Altitude_Estimate_Divergence_Result : public Analyzer_Estimate_Divergence_Result {
public:
    Analyzer_Altitude_Estimate_Divergence_Result(std::string name) :
        Analyzer_Estimate_Divergence_Result(name)
        { }
private:
};

class Analyzer_Altitude_Estimate_Divergence : public Analyzer_Estimate_Divergence {

public:

    Analyzer_Altitude_Estimate_Divergence(AnalyzerVehicle::Base *&vehicle, Data_Sources &data_sources) :
    Analyzer_Estimate_Divergence(vehicle,data_sources)
    { }

    const std::string estimate_name() const override {
        return "Altitude";
    };

    double default_delta_warn() const override { return 4.0f; }
    double default_delta_fail() const override { return 5.0f; }
    virtual uint64_t default_duration_min() const override { return 500000; }

    void end_of_log(const uint32_t packet_count) override;

    void evaluate_estimate(
        std::string name,
        AnalyzerVehicle::Altitude altitude,
        AnalyzerVehicle::Altitude oestimate);
    void evaluate() override;

    double maximum_altitude() { return _max_alt; }
    double maximum_altitude_relative() { return _max_alt_rel; }

    const std::string _config_tag() const override {
        return std::string("altitude_estimate_divergence");
    }

    const char *units() override { return "metres"; }

protected:

    Analyzer_Altitude_Estimate_Divergence_Result* new_result_object(const std::string name) override;
    virtual void open_result_add_data_sources(const std::string name) override;

private:

    double _max_alt = -1000;
    double _max_alt_rel = -1000;
    bool _was_armed = false;
    double _altitude_arm;
};

#endif
