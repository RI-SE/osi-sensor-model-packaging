/*
 * PMSF FMU Framework for FMI 2.0 Co-Simulation FMUs
 *
 * (C) 2016 -- 2018 PMSF IT Consulting Pierre R. Mai
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "OSMPDummyFunction.h"

/*
 * Debug Breaks
 *
 * If you define DEBUG_BREAKS the FMU will automatically break
 * into an attached Debugger on all major computation functions.
 * Note that the FMU is likely to break all environments if no
 * Debugger is actually attached when the breaks are triggered.
 */
#if defined(DEBUG_BREAKS) && !defined(NDEBUG)
#if defined(__has_builtin) && !defined(__ibmxl__)
#if __has_builtin(__builtin_debugtrap)
#define DEBUGBREAK() __builtin_debugtrap()
#elif __has_builtin(__debugbreak)
#define DEBUGBREAK() __debugbreak()
#endif
#endif
#if !defined(DEBUGBREAK)
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#include <intrin.h>
#define DEBUGBREAK() __debugbreak()
#else
#include <signal.h>
#if defined(SIGTRAP)
#define DEBUGBREAK() raise(SIGTRAP)
#else
#define DEBUGBREAK() raise(SIGABRT)
#endif
#endif
#endif
#else
#define DEBUGBREAK()
#endif

#include <iostream>
#include <string>
#include <algorithm>
#include <cstdint>
#include <cmath>

using namespace std;

#ifdef PRIVATE_LOG_PATH
ofstream COSMPDummyFunction::private_log_file;
#endif

/*
 * ProtocolBuffer Accessors
 */

void* decode_integer_to_pointer(fmi2Integer hi,fmi2Integer lo)
{
#if PTRDIFF_MAX == INT64_MAX
    union addrconv {
        struct {
            int lo;
            int hi;
        } base;
        unsigned long long address;
    } myaddr;
    myaddr.base.lo=lo;
    myaddr.base.hi=hi;
    return reinterpret_cast<void*>(myaddr.address);
#elif PTRDIFF_MAX == INT32_MAX
    return reinterpret_cast<void*>(lo);
#else
#error "Cannot determine 32bit or 64bit environment!"
#endif
}

void encode_pointer_to_integer(const void* ptr,fmi2Integer& hi,fmi2Integer& lo)
{
#if PTRDIFF_MAX == INT64_MAX
    union addrconv {
        struct {
            int lo;
            int hi;
        } base;
        unsigned long long address;
    } myaddr;
    myaddr.address=reinterpret_cast<unsigned long long>(ptr);
    hi=myaddr.base.hi;
    lo=myaddr.base.lo;
#elif PTRDIFF_MAX == INT32_MAX
    hi=0;
    lo=reinterpret_cast<int>(ptr);
#else
#error "Cannot determine 32bit or 64bit environment!"
#endif
}

bool COSMPDummyFunction::get_fmi_sensor_view_config(osi3::SensorViewConfiguration& data)
{
    if (integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_SIZE_IDX] > 0) {
        void* buffer = decode_integer_to_pointer(integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_BASEHI_IDX],integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_BASELO_IDX]);
        normal_log("OSMP","Got %08X %08X, reading from %p ...",integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_BASEHI_IDX],integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_BASELO_IDX],buffer);
        data.ParseFromArray(buffer,integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_SIZE_IDX]);
        return true;
    } else {
        return false;
    }
}

void COSMPDummyFunction::set_fmi_sensor_view_config_request(const osi3::SensorViewConfiguration& data)
{
    data.SerializeToString(currentConfigRequestBuffer);
    encode_pointer_to_integer(currentConfigRequestBuffer->data(),integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASEHI_IDX],integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASELO_IDX]);
    integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_SIZE_IDX]=(fmi2Integer)currentConfigRequestBuffer->length();
    normal_log("OSMP","Providing %08X %08X, writing from %p ...",integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASEHI_IDX],integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASELO_IDX],currentConfigRequestBuffer->data());
    swap(currentConfigRequestBuffer,lastConfigRequestBuffer);
}

void COSMPDummyFunction::reset_fmi_sensor_view_config_request()
{
    integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_SIZE_IDX]=0;
    integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASEHI_IDX]=0;
    integer_vars[FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASELO_IDX]=0;
}

bool COSMPDummyFunction::get_fmi_sensor_data_in(osi3::SensorData& data)
{
    if (integer_vars[FMI_INTEGER_SENSORDATA_IN_SIZE_IDX] > 0) {
        void* buffer = decode_integer_to_pointer(integer_vars[FMI_INTEGER_SENSORDATA_IN_BASEHI_IDX],integer_vars[FMI_INTEGER_SENSORDATA_IN_BASELO_IDX]);
        normal_log("OSMP","Got %08X %08X, reading from %p ...",integer_vars[FMI_INTEGER_SENSORDATA_IN_BASEHI_IDX],integer_vars[FMI_INTEGER_SENSORDATA_IN_BASELO_IDX],buffer);
        data.ParseFromArray(buffer,integer_vars[FMI_INTEGER_SENSORDATA_IN_SIZE_IDX]);
        return true;
    } else {
        return false;
    }
}

void COSMPDummyFunction::set_fmi_traffic_update_out(const osi3::TrafficUpdate& data)
{
    data.SerializeToString(currentOutputBuffer);
    encode_pointer_to_integer(currentOutputBuffer->data(),integer_vars[FMI_INTEGER_TRAFFICUPDATE_OUT_BASEHI_IDX],integer_vars[FMI_INTEGER_TRAFFICUPDATE_OUT_BASELO_IDX]);
    integer_vars[FMI_INTEGER_TRAFFICUPDATE_OUT_SIZE_IDX]=(fmi2Integer)currentOutputBuffer->length();
    normal_log("OSMP","Providing %08X %08X, writing from %p ...",integer_vars[FMI_INTEGER_TRAFFICUPDATE_OUT_BASEHI_IDX],integer_vars[FMI_INTEGER_TRAFFICUPDATE_OUT_BASELO_IDX],currentOutputBuffer->data());
    swap(currentOutputBuffer,lastOutputBuffer);
}

void COSMPDummyFunction::reset_fmi_traffic_update_out()
{
    integer_vars[FMI_INTEGER_TRAFFICUPDATE_OUT_SIZE_IDX]=0;
    integer_vars[FMI_INTEGER_TRAFFICUPDATE_OUT_BASEHI_IDX]=0;
    integer_vars[FMI_INTEGER_TRAFFICUPDATE_OUT_BASELO_IDX]=0;
}

void COSMPDummyFunction::refresh_fmi_sensor_view_config_request()
{
    osi3::SensorViewConfiguration config;
    if (get_fmi_sensor_view_config(config))
        set_fmi_sensor_view_config_request(config);
    else {
        config.Clear();
        config.mutable_version()->CopyFrom(osi3::InterfaceVersion::descriptor()->file()->options().GetExtension(osi3::current_interface_version));
        config.set_field_of_view_horizontal(3.14);
        config.set_field_of_view_vertical(3.14);
        config.set_range(fmi_nominal_range()*1.1);
        config.mutable_update_cycle_time()->set_seconds(0);
        config.mutable_update_cycle_time()->set_nanos(20000000);
        config.mutable_update_cycle_offset()->Clear();
        osi3::GenericSensorViewConfiguration* generic = config.add_generic_sensor_view_configuration();
        generic->set_field_of_view_horizontal(3.14);
        generic->set_field_of_view_vertical(3.14);
        set_fmi_sensor_view_config_request(config);
    }
}

/*
 * Actual Core Content
 */

fmi2Status COSMPDummyFunction::doInit()
{
    DEBUGBREAK();

    /* Booleans */
    for (int i = 0; i<FMI_BOOLEAN_VARS; i++)
        boolean_vars[i] = fmi2False;

    /* Integers */
    for (int i = 0; i<FMI_INTEGER_VARS; i++)
        integer_vars[i] = 0;

    /* Reals */
    for (int i = 0; i<FMI_REAL_VARS; i++)
        real_vars[i] = 0.0;

    /* Strings */
    for (int i = 0; i<FMI_STRING_VARS; i++)
        string_vars[i] = "";

    set_fmi_nominal_range(135.0);
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::doStart(fmi2Boolean toleranceDefined, fmi2Real tolerance, fmi2Real startTime, fmi2Boolean stopTimeDefined, fmi2Real stopTime)
{
    DEBUGBREAK();

    return fmi2OK;
}

fmi2Status COSMPDummyFunction::doEnterInitializationMode()
{
    DEBUGBREAK();

    return fmi2OK;
}

fmi2Status COSMPDummyFunction::doExitInitializationMode()
{
    DEBUGBREAK();

    osi3::SensorViewConfiguration config;
    if (!get_fmi_sensor_view_config(config))
        normal_log("OSI","Received no valid SensorViewConfiguration from Simulation Environment, assuming everything checks out.");
    else {
        normal_log("OSI","Received SensorViewConfiguration for Sensor Id %llu",config.sensor_id().value());
        normal_log("OSI","SVC Ground Truth FoV Horizontal %f, FoV Vertical %f, Range %f",config.field_of_view_horizontal(),config.field_of_view_vertical(),config.range());
        normal_log("OSI","SVC Mounting Position: (%f, %f, %f)",config.mounting_position().position().x(),config.mounting_position().position().y(),config.mounting_position().position().z());
        normal_log("OSI","SVC Mounting Orientation: (%f, %f, %f)",config.mounting_position().orientation().roll(),config.mounting_position().orientation().pitch(),config.mounting_position().orientation().yaw());
    }

    return fmi2OK;
}


/*!
     * \brief 3D rotation matrix function. Calculates local X,Y,Z coordinates relative from reference.  
     * \param x Global relative x-coordinate.
     * \param y Global relative y-coordinate.
     * \param z Global relative z-coordinate
     * \param yaw Global yaw reference of rotation.
     * \param pitch Global pitch reference of rotation.
     * \param roll Global roll reference of rotation.
     * \param rx Local x-coordinate after rotation. 
     * \param ry Local y-coordinate after rotation.
     * \param rz Local z-coordinate after rotation.
     */ 
void rotatePoint3D(double x, double y, double z,double yaw,double pitch,double roll,double &rx,double &ry,double &rz) // TODO: Find out why 3D rotation doesn't work. Using 2D rotation instead.
{
    double matrix[3][3];
    double cos_yaw = cos(yaw*M_PI/180);
    double cos_pitch = cos(pitch*M_PI/180);
    double cos_roll = cos(roll*M_PI/180);
    double sin_yaw = sin(yaw*M_PI/180);
    double sin_pitch = sin(pitch*M_PI/180);
    double sin_roll = sin(roll*M_PI/180);

    matrix[0][0] = cos_yaw*cos_pitch;  matrix[0][1]=cos_yaw*sin_pitch*sin_roll - sin_yaw*cos_roll; matrix[0][2]=cos_yaw*sin_pitch*cos_roll + sin_yaw*sin_roll;
    matrix[1][0] = sin_yaw*cos_pitch;  matrix[1][1]=sin_yaw*sin_pitch*sin_roll + cos_yaw*cos_roll; matrix[1][2]=sin_yaw*sin_pitch*cos_roll - cos_yaw*sin_roll;
    matrix[2][0] = -sin_pitch;         matrix[2][1]=cos_pitch*sin_roll;                            matrix[2][2]=cos_pitch*cos_roll;

    rx = matrix[0][0] * x + matrix[0][1] * y + matrix[0][2] * z;
    ry = matrix[1][0] * x + matrix[1][1] * y + matrix[1][2] * z;
    rz = matrix[2][0] * x + matrix[2][1] * y + matrix[2][2] * z;

}

/*!
     * \brief 2D rotation matrix function. Calculates local X,Y coordinates and relative yaw angle from reference.  
     * \param x Global relative x-coordinate.
     * \param y Global relative y-coordinate.
     * \param yaw Global yaw reference of rotation.
     * \param rx Local x-coordinate after rotation. 
     * \param ry Local y-coordinate after rotation.
     * \param ryaw Local yaw angle to reference after rotation.
     */
void rotatePoint2D(double x, double y, double yaw, double &rx, double &ry, double &ryaw)
{
    double cos_yaw = cos(yaw * M_PI/180);
    double sin_yaw = sin(yaw * M_PI/180);

    rx = cos_yaw*x + sin_yaw*y;
    ry = -sin_yaw*x + cos_yaw*y; 

    ryaw = (atan2(ry,rx) * 180/M_PI);
}

void COSMPDummyFunction::evaluateAEB(double &rel_x, double &rel_y, double &rel_z, double &rel_vx, double &rel_vy, double &rel_vz, double &rel_yaw, double &decelRequest)
{
    // Return if relative is positive. (AKA not approaching target)
    if (rel_vx >= 0){
        return;
    }

    // Calculate TTC relative to target
    double rel_distance = sqrt(rel_x*rel_x + rel_y*rel_y + rel_z*rel_z) - distanceOffset;
    double rel_v = sqrt(rel_vx*rel_vx + rel_vy*rel_vy + rel_vz*rel_vz);

    double ttc = rel_distance / rel_v;


    normal_log("OSI","Calculating TTC: %f,  at Distance: %f, for velocity: %f",ttc,rel_distance,rel_v);

    if (ttc< fbTTClimit && abs(rel_y) < collisionWidth){
        decelRequest = fbRequest;
        normal_log("OSI", "Detected vehicle within TTC limit. FULL BRAKE!");

    }
    else if (ttc< pbTTClimit && abs(rel_y) < collisionWidth) {
        decelRequest = pbRequest;
        normal_log("OSI", "Detected vehicle within TTC limit. PRE BRAKE!");
    }
}

fmi2Status COSMPDummyFunction::doCalc(fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPoint)
{
    DEBUGBREAK();

    osi3::SensorData currentIn;
    osi3::TrafficUpdate currentOut;
    double time = currentCommunicationPoint+communicationStepSize;
    normal_log("OSI","Calculating Function at %f for %f (step size %f)",currentCommunicationPoint,time,communicationStepSize);
    if (get_fmi_sensor_data_in(currentIn)) {
        
        int sv_size = currentIn.sensor_view_size();
        normal_log("OSI","Getting %i Sensorview from SensorData",sv_size);
        if (sv_size < 1) {
            normal_log("OSI", "No Sensorview data to calculate ego available");
            return fmi2OK;
        }
        // Get the simulation information needed about ego. 
        // Only ID and timestamp for now but if function requires more input in the future it can be added here.
        osi3::Identifier ego_id = currentIn.sensor_view().Get(0).global_ground_truth().host_vehicle_id();
        normal_log("OSI","Getting SensorData from hostvehicle with ID: %llu",ego_id.value());
        osi3::Timestamp stamp = currentIn.sensor_view().Get(0).global_ground_truth().timestamp();
        double stamp_s = stamp.seconds() + (stamp.nanos()/1000000000.0);
        normal_log("OSI", "Getting simulation timestamp: %i:%f", (int)floor(stamp_s/60.0), fmod(stamp_s,60.0));

        /* Clear Output */
        currentOut.Clear();
        currentOut.mutable_version()->CopyFrom(osi3::InterfaceVersion::descriptor()->file()->options().GetExtension(osi3::current_interface_version));
        /* Adjust Timestamps and Ids */
        currentOut.mutable_timestamp()->set_seconds((long long int)floor(time));
        currentOut.mutable_timestamp()->set_nanos((int)((time - floor(time))*1000000000.0));

        
        // Add host vehicle to TrafficUpdate
        // In the future this can be in a loop to apply the function to multiple vehicles but for now only ego is necessary
        osi3::MovingObject *obj = currentOut.mutable_update()->Add();
        obj->mutable_id()->set_value(ego_id.value());
        int i=1; 

        // For every target within FoV, check if AEB should be activated
        for_each(currentIn.moving_object().begin(),currentIn.moving_object().end(),
            [this,&i,&currentIn,&currentOut,&obj](const osi3::DetectedMovingObject& veh) {

                    double rel_x = veh.base().position().x();
                    double rel_y = veh.base().position().y();
                    double rel_z = veh.base().position().z();

                    double rel_vx = veh.base().velocity().x();
                    double rel_vy = veh.base().velocity().y();
                    double rel_vz = veh.base().velocity().z();

                    double rel_yaw = veh.base().orientation().yaw();

                    double decelRequest = 0.0;    
                    evaluateAEB(rel_x, rel_y, rel_z, rel_vx, rel_vy, rel_vz, rel_yaw, decelRequest);
                    
                    obj->mutable_base()->mutable_acceleration()->set_x(decelRequest);
                    obj->mutable_base()->mutable_acceleration()->set_y(0.0);
                    obj->mutable_base()->mutable_acceleration()->set_z(0.0);
                    obj->mutable_base()->mutable_orientation()->set_yaw(0.0);
    
            });
        normal_log("OSI","Mapped %d vehicles to output", i);
        /* Serialize */
        set_fmi_traffic_update_out(currentOut);
        set_fmi_valid(true);
        set_fmi_count(currentOut.update_size());
    } else {
        /* We have no valid input, so no valid output */
        normal_log("OSI","No valid input, therefore providing no valid output.");
        reset_fmi_traffic_update_out();
        set_fmi_valid(false);
        set_fmi_count(0);
    }
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::doTerm()
{
    DEBUGBREAK();

    return fmi2OK;
}

void COSMPDummyFunction::doFree()
{
    DEBUGBREAK();
}

/*
 * Generic C++ Wrapper Code
 */

COSMPDummyFunction::COSMPDummyFunction(fmi2String theinstanceName, fmi2Type thefmuType, fmi2String thefmuGUID, fmi2String thefmuResourceLocation, const fmi2CallbackFunctions* thefunctions, fmi2Boolean thevisible, fmi2Boolean theloggingOn)
    : instanceName(theinstanceName),
    fmuType(thefmuType),
    fmuGUID(thefmuGUID),
    fmuResourceLocation(thefmuResourceLocation),
    functions(*thefunctions),
    visible(!!thevisible),
    loggingOn(!!theloggingOn),
    simulation_started(false)
{
    currentOutputBuffer=new string();
    lastOutputBuffer=new string();
    currentConfigRequestBuffer=new string();
    lastConfigRequestBuffer=new string();
    loggingCategories.clear();
    loggingCategories.insert("FMI");
    loggingCategories.insert("OSMP");
    loggingCategories.insert("OSI");
}

COSMPDummyFunction::~COSMPDummyFunction()
{
    delete currentOutputBuffer;
    delete lastOutputBuffer;
    delete currentConfigRequestBuffer;
    delete lastConfigRequestBuffer;
}

fmi2Status COSMPDummyFunction::SetDebugLogging(fmi2Boolean theloggingOn, size_t nCategories, const fmi2String categories[])
{
    fmi_verbose_log("fmi2SetDebugLogging(%s)", theloggingOn ? "true" : "false");
    loggingOn = theloggingOn ? true : false;
    if (categories && (nCategories > 0)) {
        loggingCategories.clear();
        for (size_t i=0;i<nCategories;i++) {
            if (0==strcmp(categories[i],"FMI"))
                loggingCategories.insert("FMI");
            else if (0==strcmp(categories[i],"OSMP"))
                loggingCategories.insert("OSMP");
            else if (0==strcmp(categories[i],"OSI"))
                loggingCategories.insert("OSI");
        }
    } else {
        loggingCategories.clear();
        loggingCategories.insert("FMI");
        loggingCategories.insert("OSMP");
        loggingCategories.insert("OSI");
    }
    return fmi2OK;
}

fmi2Component COSMPDummyFunction::Instantiate(fmi2String instanceName, fmi2Type fmuType, fmi2String fmuGUID, fmi2String fmuResourceLocation, const fmi2CallbackFunctions* functions, fmi2Boolean visible, fmi2Boolean loggingOn)
{
    COSMPDummyFunction* myc = new COSMPDummyFunction(instanceName,fmuType,fmuGUID,fmuResourceLocation,functions,visible,loggingOn);

    if (myc == NULL) {
        fmi_verbose_log_global("fmi2Instantiate(\"%s\",%d,\"%s\",\"%s\",\"%s\",%d,%d) = NULL (alloc failure)",
            instanceName, fmuType, fmuGUID,
            (fmuResourceLocation != NULL) ? fmuResourceLocation : "<NULL>",
            "FUNCTIONS", visible, loggingOn);
        return NULL;
    }

    if (myc->doInit() != fmi2OK) {
        fmi_verbose_log_global("fmi2Instantiate(\"%s\",%d,\"%s\",\"%s\",\"%s\",%d,%d) = NULL (doInit failure)",
            instanceName, fmuType, fmuGUID,
            (fmuResourceLocation != NULL) ? fmuResourceLocation : "<NULL>",
            "FUNCTIONS", visible, loggingOn);
        delete myc;
        return NULL;
    }
    else {
        fmi_verbose_log_global("fmi2Instantiate(\"%s\",%d,\"%s\",\"%s\",\"%s\",%d,%d) = %p",
            instanceName, fmuType, fmuGUID,
            (fmuResourceLocation != NULL) ? fmuResourceLocation : "<NULL>",
            "FUNCTIONS", visible, loggingOn, myc);
        return (fmi2Component)myc;
    }
}

fmi2Status COSMPDummyFunction::SetupExperiment(fmi2Boolean toleranceDefined, fmi2Real tolerance, fmi2Real startTime, fmi2Boolean stopTimeDefined, fmi2Real stopTime)
{
    fmi_verbose_log("fmi2SetupExperiment(%d,%g,%g,%d,%g)", toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime);
    return doStart(toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime);
}

fmi2Status COSMPDummyFunction::EnterInitializationMode()
{
    fmi_verbose_log("fmi2EnterInitializationMode()");
    return doEnterInitializationMode();
}

fmi2Status COSMPDummyFunction::ExitInitializationMode()
{
    fmi_verbose_log("fmi2ExitInitializationMode()");
    simulation_started = true;
    return doExitInitializationMode();
}

fmi2Status COSMPDummyFunction::DoStep(fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize, fmi2Boolean noSetFMUStatePriorToCurrentPointfmi2Component)
{
    fmi_verbose_log("fmi2DoStep(%g,%g,%d)", currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPointfmi2Component);
    return doCalc(currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPointfmi2Component);
}

fmi2Status COSMPDummyFunction::Terminate()
{
    fmi_verbose_log("fmi2Terminate()");
    return doTerm();
}

fmi2Status COSMPDummyFunction::Reset()
{
    fmi_verbose_log("fmi2Reset()");

    doFree();
    simulation_started = false;
    return doInit();
}

void COSMPDummyFunction::FreeInstance()
{
    fmi_verbose_log("fmi2FreeInstance()");
    doFree();
}

fmi2Status COSMPDummyFunction::GetReal(const fmi2ValueReference vr[], size_t nvr, fmi2Real value[])
{
    fmi_verbose_log("fmi2GetReal(...)");
    for (size_t i = 0; i<nvr; i++) {
        if (vr[i]<FMI_REAL_VARS)
            value[i] = real_vars[vr[i]];
        else
            return fmi2Error;
    }
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::GetInteger(const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[])
{
    fmi_verbose_log("fmi2GetInteger(...)");
    bool need_refresh = !simulation_started;
    for (size_t i = 0; i<nvr; i++) {
        if (vr[i]<FMI_INTEGER_VARS) {
            if (need_refresh && (vr[i] == FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASEHI_IDX || vr[i] == FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_BASELO_IDX || vr[i] == FMI_INTEGER_SENSORVIEW_CONFIG_REQUEST_SIZE_IDX)) {
                refresh_fmi_sensor_view_config_request();
                need_refresh = false;
            }
            value[i] = integer_vars[vr[i]];
        } else
            return fmi2Error;
    }
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::GetBoolean(const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[])
{
    fmi_verbose_log("fmi2GetBoolean(...)");
    for (size_t i = 0; i<nvr; i++) {
        if (vr[i]<FMI_BOOLEAN_VARS)
            value[i] = boolean_vars[vr[i]];
        else
            return fmi2Error;
    }
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::GetString(const fmi2ValueReference vr[], size_t nvr, fmi2String value[])
{
    fmi_verbose_log("fmi2GetString(...)");
    for (size_t i = 0; i<nvr; i++) {
        if (vr[i]<FMI_STRING_VARS)
            value[i] = string_vars[vr[i]].c_str();
        else
            return fmi2Error;
    }
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::SetReal(const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[])
{
    fmi_verbose_log("fmi2SetReal(...)");
    for (size_t i = 0; i<nvr; i++) {
        if (vr[i]<FMI_REAL_VARS)
            real_vars[vr[i]] = value[i];
        else
            return fmi2Error;
    }
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::SetInteger(const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[])
{
    fmi_verbose_log("fmi2SetInteger(...)");
    for (size_t i = 0; i<nvr; i++) {
        if (vr[i]<FMI_INTEGER_VARS)
            integer_vars[vr[i]] = value[i];
        else
            return fmi2Error;
    }
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::SetBoolean(const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[])
{
    fmi_verbose_log("fmi2SetBoolean(...)");
    for (size_t i = 0; i<nvr; i++) {
        if (vr[i]<FMI_BOOLEAN_VARS)
            boolean_vars[vr[i]] = value[i];
        else
            return fmi2Error;
    }
    return fmi2OK;
}

fmi2Status COSMPDummyFunction::SetString(const fmi2ValueReference vr[], size_t nvr, const fmi2String value[])
{
    fmi_verbose_log("fmi2SetString(...)");
    for (size_t i = 0; i<nvr; i++) {
        if (vr[i]<FMI_STRING_VARS)
            string_vars[vr[i]] = value[i];
        else
            return fmi2Error;
    }
    return fmi2OK;
}

/*
 * FMI 2.0 Co-Simulation Interface API
 */

extern "C" {

    FMI2_Export const char* fmi2GetTypesPlatform()
    {
        return fmi2TypesPlatform;
    }

    FMI2_Export const char* fmi2GetVersion()
    {
        return fmi2Version;
    }

    FMI2_Export fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn, size_t nCategories, const fmi2String categories[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->SetDebugLogging(loggingOn, nCategories, categories);
    }

    /*
    * Functions for Co-Simulation
    */
    FMI2_Export fmi2Component fmi2Instantiate(fmi2String instanceName,
        fmi2Type fmuType,
        fmi2String fmuGUID,
        fmi2String fmuResourceLocation,
        const fmi2CallbackFunctions* functions,
        fmi2Boolean visible,
        fmi2Boolean loggingOn)
    {
        return COSMPDummyFunction::Instantiate(instanceName, fmuType, fmuGUID, fmuResourceLocation, functions, visible, loggingOn);
    }

    FMI2_Export fmi2Status fmi2SetupExperiment(fmi2Component c,
        fmi2Boolean toleranceDefined,
        fmi2Real tolerance,
        fmi2Real startTime,
        fmi2Boolean stopTimeDefined,
        fmi2Real stopTime)
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->SetupExperiment(toleranceDefined, tolerance, startTime, stopTimeDefined, stopTime);
    }

    FMI2_Export fmi2Status fmi2EnterInitializationMode(fmi2Component c)
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->EnterInitializationMode();
    }

    FMI2_Export fmi2Status fmi2ExitInitializationMode(fmi2Component c)
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->ExitInitializationMode();
    }

    FMI2_Export fmi2Status fmi2DoStep(fmi2Component c,
        fmi2Real currentCommunicationPoint,
        fmi2Real communicationStepSize,
        fmi2Boolean noSetFMUStatePriorToCurrentPointfmi2Component)
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->DoStep(currentCommunicationPoint, communicationStepSize, noSetFMUStatePriorToCurrentPointfmi2Component);
    }

    FMI2_Export fmi2Status fmi2Terminate(fmi2Component c)
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->Terminate();
    }

    FMI2_Export fmi2Status fmi2Reset(fmi2Component c)
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->Reset();
    }

    FMI2_Export void fmi2FreeInstance(fmi2Component c)
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        myc->FreeInstance();
        delete myc;
    }

    /*
     * Data Exchange Functions
     */
    FMI2_Export fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->GetReal(vr, nvr, value);
    }

    FMI2_Export fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->GetInteger(vr, nvr, value);
    }

    FMI2_Export fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->GetBoolean(vr, nvr, value);
    }

    FMI2_Export fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String value[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->GetString(vr, nvr, value);
    }

    FMI2_Export fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->SetReal(vr, nvr, value);
    }

    FMI2_Export fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->SetInteger(vr, nvr, value);
    }

    FMI2_Export fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->SetBoolean(vr, nvr, value);
    }

    FMI2_Export fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String value[])
    {
        COSMPDummyFunction* myc = (COSMPDummyFunction*)c;
        return myc->SetString(vr, nvr, value);
    }

    /*
     * Unsupported Features (FMUState, Derivatives, Async DoStep, Status Enquiries)
     */
    FMI2_Export fmi2Status fmi2GetFMUstate(fmi2Component c, fmi2FMUstate* FMUstate)
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate FMUstate)
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate)
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate FMUstate, size_t *size)
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2SerializeFMUstate (fmi2Component c, fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t size)
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2DeSerializeFMUstate (fmi2Component c, const fmi2Byte serializedState[], size_t size, fmi2FMUstate* FMUstate)
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2GetDirectionalDerivative(fmi2Component c,
        const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
        const fmi2ValueReference vKnown_ref[] , size_t nKnown,
        const fmi2Real dvKnown[],
        fmi2Real dvUnknown[])
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2SetRealInputDerivatives(fmi2Component c,
        const  fmi2ValueReference vr[],
        size_t nvr,
        const  fmi2Integer order[],
        const  fmi2Real value[])
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c,
        const   fmi2ValueReference vr[],
        size_t  nvr,
        const   fmi2Integer order[],
        fmi2Real value[])
    {
        return fmi2Error;
    }

    FMI2_Export fmi2Status fmi2CancelStep(fmi2Component c)
    {
        return fmi2OK;
    }

    FMI2_Export fmi2Status fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status* value)
    {
        return fmi2Discard;
    }

    FMI2_Export fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real* value)
    {
        return fmi2Discard;
    }

    FMI2_Export fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer* value)
    {
        return fmi2Discard;
    }

    FMI2_Export fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean* value)
    {
        return fmi2Discard;
    }

    FMI2_Export fmi2Status fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String* value)
    {
        return fmi2Discard;
    }

}