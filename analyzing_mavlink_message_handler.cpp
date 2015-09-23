#include "analyzing_mavlink_message_handler.h"

void Analyzing_MAVLink_Message_Handler::end_of_log(uint32_t packet_count)
{
    _analyze->end_of_log(packet_count);
}


void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_ahrs2_t &msg) {
    _vehicle->position_estimate("AHRS2")->set_lat(T, msg.lat/10000000.0f);
    _vehicle->position_estimate("AHRS2")->set_lon(T, msg.lng/10000000.0f);
    _vehicle->altitude_estimate("AHRS2")->set_alt(T, msg.altitude);

    _vehicle->attitude_estimate("AHRS2")->set_roll(T, rad_to_deg(msg.roll));
    _vehicle->attitude_estimate("AHRS2")->set_pitch(T, rad_to_deg(msg.pitch));
    _vehicle->attitude_estimate("AHRS2")->set_yaw(T, rad_to_deg(msg.yaw));

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_attitude_t &msg) {
    _vehicle->set_T(T);

    _vehicle->attitude_estimate("ATTITUDE")->set_roll(T, rad_to_deg(msg.roll));
    _vehicle->attitude_estimate("ATTITUDE")->set_pitch(T, rad_to_deg(msg.pitch));
    _vehicle->attitude_estimate("ATTITUDE")->set_yaw(T, rad_to_deg(msg.yaw));

    _vehicle->set_roll(rad_to_deg(msg.roll));
    _vehicle->set_pitch(rad_to_deg(msg.pitch));
    _vehicle->set_yaw(rad_to_deg(msg.yaw));

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_global_position_int_t &msg) {
    _vehicle->position_estimate("GLOBAL_POSITION_INT")->set_lat(T, msg.lat/10000000.0f);
    _vehicle->position_estimate("GLOBAL_POSITION_INT")->set_lon(T, msg.lon/10000000.0f);
    _vehicle->altitude_estimate("GLOBAL_POSITION_INT")->set_alt(T, msg.alt/1000.0f);

    // GLOBAL_POSITION_INT is the "offical" position of the vehicle:
    _vehicle->set_T(T);
    _vehicle->set_lat(msg.lat/10000000.0f);
    _vehicle->set_lon(msg.lon/10000000.0f);
    // and I'm assuming for ALT as well:
    _vehicle->set_altitude(msg.alt/1000.0f);

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_gps_raw_int_t &msg) {
    _vehicle->position_estimate("GPS_RAW_INT")->set_lat(T, msg.lat/10000000.0f);
    _vehicle->position_estimate("GPS_RAW_INT")->set_lon(T, msg.lon/10000000.0f);
    _vehicle->altitude_estimate("GPS_RAW_INT")->set_alt(T, msg.alt/1000.0f);

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_heartbeat_t &msg) {
    _vehicle->set_T(T);

    if (msg.autopilot == 8) {
        // drop any invalid-autopilot message; this could possibly be moved into
        // all callees
        return;
    }

    _vehicle->set_armed(msg.base_mode & MAV_MODE_FLAG_SAFETY_ARMED);

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_nav_controller_output_t &msg) {
    _vehicle->set_T(T);

    _vehicle->set_desroll(msg.nav_roll);
    _vehicle->set_despitch(msg.nav_pitch);
    _vehicle->set_desyaw(msg.nav_bearing);

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_param_value_t &msg) {
    _vehicle->set_T(T);

    char buf[17] = { };
    memcpy(buf, msg.param_id, 16);
    _vehicle->param_set(buf, msg.param_value);

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_servo_output_raw_t &msg) {
    _vehicle->set_T(T);

    _vehicle->set_servo_output(msg.servo1_raw,
                              msg.servo2_raw,
                              msg.servo3_raw,
                              msg.servo4_raw,
                              msg.servo5_raw,
                              msg.servo6_raw,
                              msg.servo7_raw,
                              msg.servo8_raw);

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_ekf_status_report_t &msg) {
    _vehicle->set_T(T);

    _vehicle->ekf_set_variance("velocity", msg.velocity_variance);
    _vehicle->ekf_set_variance("pos_horiz", msg.pos_horiz_variance);
    _vehicle->ekf_set_variance("pos_vert", msg.pos_vert_variance);
    _vehicle->ekf_set_variance("compass", msg.compass_variance);
    _vehicle->ekf_set_variance("terrain_alt", msg.terrain_alt_variance);

    _vehicle->ekf_set_flags(msg.flags);

    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_sys_status_t &msg) {
    _vehicle->set_T(T);

    _vehicle->set_battery_remaining(msg.battery_remaining);

    for (std::map<const std::string, const uint64_t>::const_iterator it = _sensor_masks.begin();
         it != _sensor_masks.end();
         it++) {
        std::string name = (*it).first;
        uint64_t mask = (*it).second;
        // ::fprintf(stderr, "sensor: %s\n", name.c_str());
        // _vehicle->sensor_present(name, msg.onboard_control_sensors_present & mask);
        // _vehicle->sensor_enabled(name, msg.onboard_control_sensors_enabled & mask);
        // _vehicle->sensor_healthy(name, msg.onboard_control_sensors_health & mask);
        if (msg.onboard_control_sensors_present & mask &&
            msg.onboard_control_sensors_enabled & mask) {
            _vehicle->sensor_set_healthy(name, msg.onboard_control_sensors_health & mask);
        }
    }
    
    _analyze->evaluate_all();
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_vfr_hud_t &msg) {
    _vehicle->set_T(T);

    // vfr_hud is a relative altitude, set_alt sets absolute altitude
//    _vehicle->set_alt(msg.alt);

    // _analyze->evaluate_all();
}

// something like this is in analyzing_dataflash_mavlink_message_handler
void Analyzing_MAVLink_Message_Handler::set_vehicle_copter()
{
    AnalyzerVehicle::Base *vehicle_old = _vehicle;
    AnalyzerVehicle::Copter *vehicle_new = new AnalyzerVehicle::Copter();
    vehicle_new->take_state(vehicle_old);
    _vehicle = vehicle_new;
    delete vehicle_old;
}

void Analyzing_MAVLink_Message_Handler::handle_decoded_message(uint64_t T, mavlink_statustext_t &msg) {
    _vehicle->set_T(T);

    if (strstr(msg.text, "APM:Copter") || strstr(msg.text, "ArduCopter")) {
        set_vehicle_copter();
    }

    switch (_vehicle->vehicletype()) {
    case AnalyzerVehicle::Base::vehicletype_t::copter:
        if (strstr(msg.text, "Frame")) {
            ((AnalyzerVehicle::Copter*&)_vehicle)->set_frame(msg.text);
        }
        break;
    case AnalyzerVehicle::Base::vehicletype_t::invalid:
        break;
    }
}
