#include "scrolling_interpolator.h"

#include <algorithm>


// ParabolicInterpolator
// 加速度
const float kAcceleration = 1000;


// ParabolicInterpolator
int ParabolicInterpolator::GetScrollingSpan(
    int time_interval, int total_distance, float scale) {

    int sign = 1;
    if (total_distance == 0) {
        return 0;
    } else if (total_distance < 0) {
        sign = -1;
        total_distance = std::abs(total_distance);
    }

    float cur_velocity = std::sqrt(2 * kAcceleration * scale * total_distance);
    float cur_distance = cur_velocity * time_interval / 1000;

    return std::ceil(cur_distance) * sign;
}


// CubicBezierInterpolator
CubicBezierInterpolator::CubicBezierInterpolator() {
    float deceleration_rate = std::log(0.78f) / std::log(0.9f); // 谜之参数

    inflexion_ = 0.35f; // 贝塞尔曲线 P2 点的横坐标
    friction_factor_ = 0.003f; // 摩擦系数
    deceleration_factor_ = (deceleration_rate - 1) / deceleration_rate;
    gravity_acceleration_ =
        9.80665f // 重力加速度，单位 m/s^2
        * 100    // 单位 cm/s^2
        / 2.54f  // 单位 inch/s^2
        * 96     // 单位 px/s^2 (标准屏幕)
        * 0.4f;  // 调整系数，试验得到
}

int CubicBezierInterpolator::GetScrollingSpan(
    int time_interval, int total_distance, float scale) {

    if (scale < 1.f) {
        scale = 1.f;
    }

    int sign = 1;
    if (total_distance == 0) {
        return 0;
    } else if (total_distance < 0) {
        sign = -1;
        total_distance = std::abs(total_distance);
    }

    float physical_coef = friction_factor_ * gravity_acceleration_ * scale;
    float factor1 = physical_coef / inflexion_;
    float factor2 = total_distance / physical_coef;

    float cur_velocity = factor1 * std::pow(factor2, deceleration_factor_);
    float cur_distance = cur_velocity * time_interval / 1000;

    return std::ceil(cur_distance) * sign;
}