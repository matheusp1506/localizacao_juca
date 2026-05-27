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

Projeto ainda em estajo de desenvolvimento, informações abaixo são referentes à um modelo de estrutura para o GitHub e não são pertinentes ao projeto.

Projeto foi implementado com o nRF OpenConnect SDK versão 2.4.x.
Consulte `Configuring your application <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/modifying.html#configure-application>`_ para obter informações sobre como alterar a configuração permanente ou temporariamente.

A configuração do perfil do dispositivo é realizada no arquivo de configuração `prj.conf <prj.conf>`_:

- End-Device:

.. code:: C 

  (...)
  CONFIG_ZIGBEE_ROLE_END_DEVICE=y  
  # CONFIG_ZIGBEE_ROLE_ROUTER=y
  // Versão do firmware
  CONFIG_MCUBOOT_IMAGE_VERSION="0.0.3"
  CONFIG_ZIGBEE_FOTA_COMMENT="ruido_zigbee_endpoint"
  # CONFIG_ZIGBEE_FOTA_COMMENT="ruido_zigbee_router"


Interface do usuário
********************

LED 1:
  Pisca enquanto o filtro estiver ativo.

Botão 1:
  Ativa o módulo xyz.


Compilando e executando
***********************

Colocar detalhes na construção da applicação. Exemplo: 

Para compilar o projeto com o Visual Studio Code, siga as etapas listadas na página `How to build an application <https://nrfconnect.github.io/vscode-nrf-connect/get_started/build_app_ncs.html>`_  na documentação da extensão nRF Connect for VS Code.  `Building and programming an application  <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/programming.html#gs-programming>`_ para outros cenários de construção e programação e `Testing and debugging an application <https://developer.nordicsemi.com/nRF_Connect_SDK/doc/2.4.2/nrf/getting_started/testing.html#gs-testing>`_ para obter informações gerais sobre testes e depuração no nRF Connect SDK.

Recomenda-se o uso do J-Link para gravação e/ou depuração.

Testando
========

Após programar o microcontrolador, conclua as etapas a seguir para testá-lo:

1. ...

Montagem
********

Breve descrição da montagem final do projeto.

