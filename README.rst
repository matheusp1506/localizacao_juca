Visualização de Localização do Robô Juca
#####################################################################

.. contents::
   :local:
   :depth: 2



Requisitos
**********

Este projeto foi implementado com os seguintes módulos/softwares/hardwares...

- `ROS 2 <https://www.ros.org/>`_
- `MQTT Client for ROS 2 <https://wiki.ros.org/mqtt_client>`_
- `MQTT Broker <https://mosquitto.org/>`_
- `MPU-6050 <https://product.tdk.com/en/search/sensor/mortion-inertial/imu/info?part_no=MPU-6050>`_
- `ESP32-S3 <https://www.espressif.com/en/products/socs/esp32-s3>`_
- `Gazebo <https://gazebosim.org/docs/latest/getstarted/>`_


Visão geral
***********

Esse projeto faz parte do desenvolvimento do robô Juca, utilizando os módulos já presentes na estrutura do robô para permitir visualização conforme o movimento do robô.

O desenvolvimento foi divido em quatro etapas:

- Etapa 1 (02 de Abril de 2026): Estudos sobre o ROS, comunicação do microcontrolador, odometria e testes de visualização no RViz.
- Etapa 2 (30 de Abril de 2026): Testes de bibliotecas no ESP32 e construção inicial de estimativa de posição.
- Etapa 3 (28 de Maio de 2026):  Continuação da estimativa de posição e visualização no Gazebo
- Etapa 4 (09 de Julho de 2026): (breve resumo da etapa)

Configuração
*************

Projeto ainda em estágio de desenvolvimento, informações abaixo estão sugeitas à mudanças.

O projeto é implementado utilizando o ESP-IDF v5.3.1 em um ESP32-S3 e o ROS 2 Kilted Kaiju em um computador com o Gazebo referente a versão do ROS.

Para o uso correto dos códigos apresentados, é importante a mudança em ``mqtt_manager.h`` e ``wifi_manager.c`` dos dados para os IPs necessários e rede utilizada, assim como em ``mqtt_bridge.py`` e versões variantes, como a com EKF. Modificando os seguintes:

- mqtt_manager:

.. code:: C 

  (...)
  #define MQTT_BROKER_URI      "mqtt://localhost:1883"
  (...)

- wifi_manager:

.. code:: C 

  (...)
  #define WIFI_SSID      "SSID"
  #define WIFI_PASS      "PASS"
  (...)

- mqtt_bridge:

.. code:: python

  (...)
  self.mqtt_client.connect("localhost", 1883)
  (...)

Interface do usuário
********************

Simulação do Gazebo:
  Exibe o movimento atual recebido.

Logs:
  Apresenta os dados recebidos.

Console:
  Permite a visualização instantânea de dados recebidos via MQTT.

Página por meio de ``visualize.py``:
   Permite a visualização de caminhos anteriores.


Compilando e executando
***********************

Para a compilação utiliza-se o ESP-IDF v5.3.1 com o target de ESP32-S3, o código é então carregado ao robô. Para as partes no computador certifique-se de utilizar o Python 3 e baixar as bibliotecas apresentadas para cada script (como ``paho-mqtt``, para as pontes e ``plotly``, para a visualização de logs), além de ter o CLI utilizado já com o ROS 2 habilitado, utilizando sua versão Kilted Kaiju.

Em cada etapa, há um arquivo ``Tests.rst`` que apresenta como instalar e testar os componentes necessários de cada conjunto de scripts.

Testando
========

Após programar o microcontrolador, conclua as etapas a seguir para testá-lo:

1. Verifique se o robô se move após 5 segundos ligado, com a energia da bateria sendo utilizada.
2. Verifique por meio da ponte o recebimento de dados via MQTT, ou se inscrevendo no tópico de maneira externa.
3. Inscreva-se nos tópicos do ROS 2 para a função desejada e observe o recebimento de dados.
4. Observe por meio do Gazebo o movimento do robô ou a mudança de valores em seu componente Pose.

