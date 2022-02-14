/*
 * PMSF FMU Framework for FMI 2.0 Co-Simulation FMUs
 *
 * (C) 2016 -- 2018 PMSF IT Consulting Pierre R. Mai
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

using namespace std;

#ifndef FMU_SHARED_OBJECT
#define FMI2_FUNCTION_PREFIX OSMPDummyFunction_
#endif
#include "fmi2Functions.h"

/*
 * Logging Control
 *
 * Logging is controlled via three definitions:
 *
 * - If PRIVATE_LOG_PATH is defined it gives the name of a file
 *   that is to be used as a private log file.
 * - If PUBLIC_LOGGING is defined then we will (also) log to
 *   the FMI logging facility where appropriate.
 * - If VERBOSE_FMI_LOGGING is defined then logging of basic
 *   FMI calls is enabled, which can get very verbose.
 */

/*
 * Variable Definitions
 *
 * Define FMI_*_LAST_IDX to the zero-based index of the last variable
 * of the given type (0 if no variables of the type exist).  This
 * ensures proper space allocation, initialisation and handling of
 * the given variables in the template code.  Optionally you can
 * define FMI_TYPENAME_VARNAME_IDX definitions (e.g. FMI_REAL_MYVAR_IDX)
 * to refer to individual variables inside your code, or for example
 * FMI_REAL_MYARRAY_OFFSET and FMI_REAL_MYARRAY_SIZE definitions for
 * array variables.
 */

/* Boolean Variables */
#define FMI_BOOLEAN_VALID_IDX 0
#define FMI_BOOLEAN_LAST_IDX FMI_BOOLEAN_VALID_IDX
#define FMI_BOOLEAN_VARS (FMI_BOOLEAN_LAST_IDX+1)

/* Integer Variables */
#define FMI_INTEGER_SENSORDATA_IN_BASELO_IDX 0
#define FMI_INTEGER_SENSORDATA_IN_BASEHI_IDX 1
#define FMI_INTEGER_SENSORDATA_IN_SIZE_IDX 2
#define FMI_INTEGER_TRAFFICUPDATE_OUT_BASELO_IDX 3
#define FMI_INTEGER_TRAFFICUPDATE_OUT_BASEHI_IDX 4
#define FMI_INTEGER_TRAFFICUPDATE_OUT_SIZE_IDX 5
#define FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASELO_IDX 6
#define FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASEHI_IDX 7
#define FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_SIZE_IDX 8
#define FMI_INTEGER_SENSORVIEW_CONFIG_BASELO_IDX 9
#define FMI_INTEGER_SENSORVIEW_CONFIG_BASEHI_IDX 10
#define FMI_INTEGER_SENSORVIEW_CONFIG_SIZE_IDX 11
#define FMI_INTEGER_COUNT_IDX 12
#define FMI_INTEGER_LAST_IDX FMI_INTEGER_COUNT_IDX
#define FMI_INTEGER_VARS (FMI_INTEGER_LAST_IDX+1)

/* Real Variables */
#define FMI_REAL_NOMINAL_RANGE_IDX 0
#define FMI_REAL_LAST_IDX FMI_REAL_NOMINAL_RANGE_IDX
#define FMI_REAL_VARS (FMI_REAL_LAST_IDX+1)

/* String Variables */
#define FMI_STRING_LAST_IDX 0
#define FMI_STRING_VARS (FMI_STRING_LAST_IDX+1)

#include <iostream>
#include <fstream>
#include <string>
#include <cstdarg>
#include <set>

#undef min
#undef max
#include "osi_sensordata.pb.h"
#include "osi_trafficupdate.pb.h"

/* FMU Class */
class COSMPDummyFunction {
public:
    /* FMI2 Interface mapped to C++ */
    COSMPDummyFunction(fmi2String theinstanceName, fmi2Type thefmuType, fmi2String thefmuGUID, fmi2String thefmuResourceLocation, const fmi2CallbackFunctions* thefunctions, fmi2Boolean thevisible, fmi2Boolean theloggingOn);
    ~COSMPDummyFunction();
    fmi2Status SetDebugLogging(fmi2Boolean theloggingOn,size_t nCategories, const fmi2String categories[]);
    static fmi2Component Instantiate(fmi2String instanceName, fmi2Type fmuType, fmi2String fmuGUID, fmi2String fmuResourceLocation, const fmi2CallbackFunctions* functions, fmi2Boolean visible, fmi2Boolean loggingOn);
    fmi2Status SetupExperiment(fmi2Boolean toleranceDefined, fmi2Real tolerance, fmi2Real startTime, fmi2Boolean stopTimeDefined, fmi2Real stopTime);
    fmi2Status EnterInitializationMode();
    fmi2Status ExitInitializationMode();
    fmi2Status DoStep(fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPointfmi2Component);
    fmi2Status Terminate();
    fmi2Status Reset();
    void FreeInstance();
    fmi2Status GetReal(const fmi2ValueReference vr[], size_t nvr, fmi2Real value[]);
    fmi2Status GetInteger(const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[]);
    fmi2Status GetBoolean(const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[]);
    fmi2Status GetString(const fmi2ValueReference vr[], size_t nvr, fmi2String value[]);
    fmi2Status SetReal(const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[]);
    fmi2Status SetInteger(const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[]);
    fmi2Status SetBoolean(const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[]);
    fmi2Status SetString(const fmi2ValueReference vr[], size_t nvr, const fmi2String value[]);

protected:
    /* Internal Implementation */
    fmi2Status doInit();
    fmi2Status doStart(fmi2Boolean toleranceDefined, fmi2Real tolerance, fmi2Real startTime, fmi2Boolean stopTimeDefined, fmi2Real stopTime);
    fmi2Status doEnterInitializationMode();
    fmi2Status doExitInitializationMode();
    fmi2Status doCalc(fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPointfmi2Component);
    fmi2Status doTerm();
    void doFree();

protected:
    /* Private File-based Logging just for Debugging */
#ifdef PRIVATE_LOG_PATH
    static ofstream private_log_file;
#endif

    static void fmi_verbose_log_global(const char* format, ...) {
#ifdef VERBOSE_FMI_LOGGING
#ifdef PRIVATE_LOG_PATH
        va_list ap;
        va_start(ap, format);
        char buffer[1024];
        if (!private_log_file.is_open())
            private_log_file.open(PRIVATE_LOG_PATH, ios::out | ios::app);
        if (private_log_file.is_open()) {
#ifdef _WIN32
            vsnprintf_s(buffer, 1024, format, ap);
#else
            vsnprintf(buffer, 1024, format, ap);
#endif
            private_log_file << "OSMPDummyFunction" << "::Global:FMI: " << buffer << endl;
            private_log_file.flush();
        }
#endif
#endif
    }

    void internal_log(const char* category, const char* format, va_list arg)
    {
#if defined(PRIVATE_LOG_PATH) || defined(PUBLIC_LOGGING)
        char buffer[1024];
#ifdef _WIN32
        vsnprintf_s(buffer, 1024, format, arg);
#else
        vsnprintf(buffer, 1024, format, arg);
#endif
#ifdef PRIVATE_LOG_PATH
        if (!private_log_file.is_open())
            private_log_file.open(PRIVATE_LOG_PATH, ios::out | ios::app);
        if (private_log_file.is_open()) {
            private_log_file << "OSMPDummyFunction" << "::" << instanceName << "<" << ((void*)this) << ">:" << category << ": " << buffer << endl;
            private_log_file.flush();
        }
#endif
#ifdef PUBLIC_LOGGING
        if (loggingOn && loggingCategories.count(category))
            functions.logger(functions.componentEnvironment,instanceName.c_str(),fmi2OK,category,buffer);
#endif
#endif
    }

    void fmi_verbose_log(const char* format, ...) {
#if  defined(VERBOSE_FMI_LOGGING) && (defined(PRIVATE_LOG_PATH) || defined(PUBLIC_LOGGING))
        va_list ap;
        va_start(ap, format);
        internal_log("FMI",format,ap);
        va_end(ap);
#endif
    }

    /* Normal Logging */
    void normal_log(const char* category, const char* format, ...) {
#if defined(PRIVATE_LOG_PATH) || defined(PUBLIC_LOGGING)
        va_list ap;
        va_start(ap, format);
        internal_log(category,format,ap);
        va_end(ap);
#endif
    }

protected:
    /* Members */
    string instanceName;
    fmi2Type fmuType;
    string fmuGUID;
    string fmuResourceLocation;
    bool visible;
    bool loggingOn;
    set<string> loggingCategories;
    fmi2CallbackFunctions functions;
    fmi2Boolean boolean_vars[FMI_BOOLEAN_VARS];
    fmi2Integer integer_vars[FMI_INTEGER_VARS];
    fmi2Real real_vars[FMI_REAL_VARS];
    string string_vars[FMI_STRING_VARS];
    bool simulation_started;
    string* currentOutputBuffer;
    string* lastOutputBuffer;
    string* currentConfigRequestBuffer;
    string* lastConfigRequestBuffer;

    /* Simple Accessors */
    fmi2Boolean fmi_valid() { return boolean_vars[FMI_BOOLEAN_VALID_IDX]; }
    void set_fmi_valid(fmi2Boolean value) { boolean_vars[FMI_BOOLEAN_VALID_IDX]=value; }
    fmi2Integer fmi_count() { return integer_vars[FMI_INTEGER_COUNT_IDX]; }
    void set_fmi_count(fmi2Integer value) { integer_vars[FMI_INTEGER_COUNT_IDX]=value; }
    fmi2Real fmi_nominal_range() { return real_vars[FMI_REAL_NOMINAL_RANGE_IDX]; }
    void set_fmi_nominal_range(fmi2Real value) { real_vars[FMI_REAL_NOMINAL_RANGE_IDX]=value; }

    
    /* Protocol Buffer Accessors */
    bool get_fmi_sensor_view_config(osi3::SensorViewConfiguration& data);
    void set_fmi_sensor_view_config_request(const osi3::SensorViewConfiguration& data);
    void reset_fmi_sensor_view_config_request();
    bool get_fmi_sensor_data_in(osi3::SensorData& data);
    void set_fmi_traffic_update_out(const osi3::TrafficUpdate& data);
    void reset_fmi_traffic_update_out();


    /* Refreshing of Calculated Parameters */
    void refresh_fmi_sensor_view_config_request();

    /* TrafficUpdate functions */
    void evaluateAEB(double &rel_x, double &rel_y, double &rel_z, double &rel_vx, double &rel_vy, double &rel_vz, double &rel_yaw, double &decelRequest);
    
    /* TrafficUpdate variables */
    double pbTTClimit = 2.5;    // Prebrake TTC limit [s] 
    double pbRequest = -3.0;    // Prebrake decel request [m/s^2]
    double fbTTClimit = 1.5;    // Fullbrake TTC limit [s] 
    double fbRequest = -5.0;    // Fullbrake decel request [m/s^2]
    // Offsets. TODO add information to Sensordata so that this could be derived in realtime from target  
    double collisionWidth = 1.5; // ~Width/2 of target (Could be derived with getting dimension of target) 
    double distanceOffset = 4.5; // ~Length/2 of target + ~length/2 of ego. (Since Sensorview calculates distance from base of ego to base of target) 
};