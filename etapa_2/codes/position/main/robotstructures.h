#ifndef ROBOTSTRUCTURES_H
#define ROBOTSTRUCTURES_H

#include "freertos/queue.h"
#include "freertos/event_groups.h"

#define WIFI_CONNECTED_BIT BIT0
// -----------------ALGUNS DEFINES DO CARRINHO----------------------------------
#define OBSTACLE_THRESHOLD_CM 20.0f
// --- CONSTANTES FÍSICAS DO JUCA ---
#define WHEEL_RADIUS    0.033f   // Raio da roda em metros (33mm)
#define TRACK_WIDTH     0.150f   // Distância entre rodas em metros (15cm)
#define ENCODER_PPR     330.0f   // Pulsos por volta total

// -----------------DEFINIÇÕES DE STRUCT E QUEUE--------------------------------
typedef struct {
    float x;
    float y;
    float quat_x;
    float quat_y;
    float quat_z;
    float quat_w;
    float linear_velocity;
    float angular_velocity;
} odom_final_data_t;

extern QueueHandle_t xOdomRosQueue; // Fila de saída

typedef struct {
    int32_t pulses_left;
    int32_t pulses_right;
    float quat_x;
    float quat_y;
	float quat_z;
	float quat_w;
    float gyro_z;          // Velocidade angular do MPU6050
    float dt;              // Tempo entre leituras
} robot_data_t;

extern QueueHandle_t xRobotDataQueue;

extern EventGroupHandle_t s_wifi_event_group;

#endif // ROBOTSTRUCTURES_H