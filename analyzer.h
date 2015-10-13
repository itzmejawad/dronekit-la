#ifndef ANALYZER_H
#define ANALYZER_H

#include <jsoncpp/json/json.h> // libjsoncpp0 and libjsoncpp-dev on debian
#include <jsoncpp/json/writer.h> // libjsoncpp0 and libjsoncpp-dev on debian

#include "analyzervehicle.h"
#include "INIReader.h"

#include "data_sources.h"

#include "analyzer_util.h"

enum analyzer_status {
    analyzer_status_warn = 17,
    analyzer_status_fail,
    analyzer_status_ok,
};

const char *_status_as_string(analyzer_status status);

class Analyzer_Result {
public:
    virtual ~Analyzer_Result() { }

    const char *status_as_string() const {
        return _status_as_string(_status);
    }
    analyzer_status status() { return _status; }
    void set_status(analyzer_status status) { _status = status; }

    void set_reason(const std::string reason) {
        if (_reason != NULL) {
            delete(_reason);
        }
        _reason = new std::string(reason);
    }
    const std::string *reason() const { return _reason; }

    virtual void to_json(Json::Value &root) const;

    void add_evidence(const std::string f) {
        _evidence.push_back(f);
    }
    // void add_series(const std::string f) {
    //     _series.push_back(f);
    // }
    void add_source(const Data_Source *f) {
        if (f == NULL) {
            abort();
        }
        _sources.push_back(f);
    }

    void add_evilness(uint32_t evilness) {
        _evilness += evilness;
    }
    void set_evilness(uint32_t evilness) {
        _evilness = evilness;
    }
    uint32_t evilness() const {
        return _evilness;
    }

private:
    analyzer_status _status = analyzer_status_ok;
    std::string *_reason = NULL;

    std::vector<std::string> _evidence;
    std::vector<const Data_Source*> _sources;

    void to_json_add_array(Json::Value &root,
                           std::string name,
                           std::vector<std::string> array) const;
    uint32_t _evilness = 0;
};

class Analyzer_Result_Period : public Analyzer_Result {
public:
    Analyzer_Result_Period() :
        Analyzer_Result()
        { }

    virtual void to_json(Json::Value &root) const override;
    
    void set_T_start(const uint64_t start) { _T_start = start; }
    uint64_t T_start() const { return _T_start; }

    void set_T_stop(const uint64_t stop) { _T_stop = stop; }
    uint64_t T_stop() const { return _T_stop; }

    uint64_t duration() const { return _T_stop - _T_start; }

private:
    uint64_t _T_start = 0;
    uint64_t _T_stop;
};

class Analyzer_Result_Summary : public Analyzer_Result {
public:
    Analyzer_Result_Summary() :
        Analyzer_Result()
        { }

    // virtual void to_json(Json::Value &root) const override;

private:
};


class Analyzer_Result_Event : public Analyzer_Result {
public:
    Analyzer_Result_Event() :
        Analyzer_Result()
        { }

    virtual void to_json(Json::Value &root) const;
    
    void set_T(uint64_t timestamp) {
        _T = timestamp;
    }

    uint64_t T() const {
        return _T;
    }

private:
    uint64_t _T;
};


class Analyzer {

public:
    Analyzer(AnalyzerVehicle::Base *&vehicle, Data_Sources &data_sources) :
        _vehicle(vehicle),
        _data_sources(data_sources)
        { }

    virtual bool configure(INIReader *config UNUSED) {
        return true;
    }

    virtual const std::string name() const = 0;
    virtual const std::string description() const = 0;
    virtual void results_json_results(Json::Value &root);
    virtual void end_of_log(uint32_t packet_count UNUSED) { }

    const char *status_as_string() {
        return _status_as_string(status());
    }

    std::vector<Analyzer_Result*> results() const {
        return _results;
    }
    uint16_t result_count() {
        return _results.size();
    }

    virtual uint32_t evilness() const;

    virtual void evaluate() { }

    virtual void add_result(Analyzer_Result* result) {
        _results.push_back(result);
    }

    analyzer_status status();

protected:
    std::string to_string(double x);

    AnalyzerVehicle::Base *&_vehicle;

    // FIXME: scope
    std::vector<Analyzer_Result*> _results;

    Data_Sources &_data_sources;

private:
};

#endif
    


// - two fundamental types of test
//  - is the software working correctly (EKF issues)
//  - is the vehicle doing sensible things (attitude etc)
