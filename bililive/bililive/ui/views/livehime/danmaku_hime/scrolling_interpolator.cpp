#include "scrolling_interpolator.h"

#include <algorithm>


// ParabolicInterpolator
// ���ٶ�
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
    float deceleration_rate = std::log(0.78f) / std::log(0.9f); // ��֮����

    inflexion_ = 0.35f; // ���������� P2 ��ĺ�����
    friction_factor_ = 0.003f; // Ħ��ϵ��
    deceleration_factor_ = (deceleration_rate - 1) / deceleration_rate;
    gravity_acceleration_ =
        9.80665f // �������ٶȣ���λ m/s^2
        * 100    // ��λ cm/s^2
        / 2.54f  // ��λ inch/s^2
        * 96     // ��λ px/s^2 (��׼��Ļ)
        * 0.4f;  // ����ϵ��������õ�
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