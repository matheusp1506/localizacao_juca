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
- `Robô Juca <https://github.com/MatheusPinto/Juca>`_
- `ESP-IDF <https://developer.espressif.com/tags/esp-idf/>`_
- `Mosquitto <https://mosquitto.org/download/>`_


Visão geral
***********

Esse projeto faz parte do desenvolvimento do robô Juca, utilizando os módulos já presentes na estrutura do robô para permitir visualização conforme o movimento do robô.

O desenvolvimento foi divido em quatro etapas:

- Etapa 1 (02 de Abril de 2026): Estudos sobre o ROS, comunicação do microcontrolador, odometria e testes de visualização no RViz.
- Etapa 2 (30 de Abril de 2026): Testes de bibliotecas no ESP32 e construção inicial de estimativa de posição.
- Etapa 3 (28 de Maio de 2026):  Continuação da estimativa de posição e visualização no Gazebo.
- Etapa 4 (09 de Julho de 2026): Testes relacionados aos valores de posição e visualização de tragetória.

Instalação
**********

Para a construção do projeto, utilize o ESP-IDF v5.3.1, este pode ser instalado por meio da UI ESP-IDF ou por CLI, para mais detalhes consulte a `documentação do ESP-IDF <https://developer.espressif.com/tags/esp-idf/>`_.

A versão utilizada do sistema operacional, para o desenvolvimento do projeto, foi Ubuntu 24.04.4 LTS, utilizada por meio do WSL 2.

Para a versão do ROS 2, utilize a Kilted Kaiju. Para mais detalhes consulte a `documentação do ROS 2 <https://docs.ros.org/en/kilted/Installation.html>`_.

Instale o broker MQTT, por exemplo, o `Mosquitto <https://mosquitto.org/download/>`_.

Instale as dependências do projeto, entre elas: `paho-mqtt <https://pypi.org/project/paho-mqtt/>`_.

Para o Gazebo, utilize a versão recomendada para o ROS 2 Kilted Kaiju, encontrado no processo de `instalação do Gazebo <https://gazebosim.org/docs/latest/ros_installation/>`_.

Crie uma pasta para o projeto, colocando dentro dela o arquivo ``juca.urdf`` e uma pasta ``src``, para os scripts do ROS. A pasta para o firmware do ESP32 pode ser outra de sua preferência.

Na pasta ``src``, coloque os arquivos responsáveis por cada parte do projeto, ``ekf.yaml``, ``ekf_to_gazebo.py``, ``juca_sim.launch.py``, ``mqtt_bridge_ekf.py``, ``log_visualize.py``. Caso deseje realizar um teste sem o robô, utilize o código ``mock_robot.py`` em conjunto com os outros scripts. 

Configuração
*************

O projeto é implementado utilizando o ESP-IDF v5.3.1 em um ESP32-S3 e o ROS 2 Kilted Kaiju em um computador com o Gazebo referente a versão do ROS.

Para o uso correto dos códigos apresentados, é importante a mudança em ``mqtt_manager.h`` e ``wifi_manager.c`` dos dados para os IPs necessários e rede utilizada, assim como em ``mqtt_bridge_ekf.py``, além destes no código ``test_tasks/wheel_task.c`` colocar o movimento desejado do robô. Modificando os seguintes:

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

- mqtt_bridge_ekf.py:

.. code:: python

  (...)
  self.mqtt_client.connect("localhost", 1883)
  (...)

Interface do usuário
********************

Simulação do Gazebo:
  Exibe a representação do robô no ambiente virtual e seu movimento, baseado nos dados enviados pela movimentação do robô.

Logs:
  Apresenta os dados que foram recebidos, permitindo a utilização deles para uma noção de seus movimentos anteriores.

Console:
  Permite a visualização instantânea de dados recebidos via MQTT e visualização de dados resultantes do filtro.

Página por meio de ``log_visualize.py``:
   Permite a visualização de caminhos anteriores, selecionando um log específico para em uma página poder ver com variação da quantidade de pontos.


Compilando e executando
***********************

Para a compilação utiliza-se o ESP-IDF v5.3.1 com o target de ESP32-S3, o código é então carregado ao robô. Para as partes no computador certifique-se de utilizar o Python 3 e baixar as bibliotecas apresentadas para cada script (como ``paho-mqtt``, para as pontes), além de ter o CLI utilizado já com o ROS 2 habilitado, utilizando sua versão Kilted Kaiju, acima, em Instalação, se encontra o procedimento.

Nas três primeiras etapas do projeto, foi desenvolvido um arquivo ``Tests.rst`` que apresenta como instalar e testar os componentes necessários de cada conjunto de scripts. Para a quarta etapa não foi elaborado um arquivo deste tipo, como representa o estado final do projeto.

Para o uso do código completo, segue-se:

1.  Mantenha o broker MQTT ativo e o simulador do Gazebo rodando (via ``juca_sim.launch.py``).
2.  Carregue o código na pasta `ESP <etapa_4/codes/esp>`_ no robô para obter os dados.
3.  Inicie o script responsável por receber os dados dos sensores via MQTT e aplicar o algoritmo de filtragem EKF:
    
    .. code-block:: bash

       python3 <YOUR_PATH>/src/mqtt_bridge_ekf.py

4.  Execute o nó ou script encarregado de pegar a saída tratada pelo filtro EKF e aplicá-la ao robô dentro do simulador:
    
    .. code-block:: bash

       python3 <YOUR_PATH>/src/ekf_to_gazebo.py

5.  Utilize a ferramenta de *parameter bridge* do ROS 2 para mapear o comando de alteração de pose (``set pose``):

    .. code-block:: bash

      ros2 run ros_gz_bridge parameter_bridge /world/empty/set_pose@ros_gz_interfaces/srv/SetEntityPose

6.  Execute o **robot_localization** usando o filtro definido:

    .. code-block:: bash

      ros2 run robot_localization ekf_node --ros-args --params-file <YOUR_PATH>/src/ekf.yaml

Caso deseje-se visualizar uma tragetória, só é nescessário rodar o script ``log_visualize.py`` e selecionar o log desejado. Caso o sistema utilizado não possua nenhum navegador reconhecido, é possível encontrar na mesma pasta do script o html gerado para a tragetória.

Testando
========

Após programar o microcontrolador, conclua as etapas a seguir para testá-lo:

1. Verifique se o robô se move após 5 segundos ligado, com a energia da bateria sendo utilizada.
2. Verifique por meio da ponte o recebimento de dados via MQTT, ou se inscrevendo no tópico de maneira externa.
3. Inscreva-se nos tópicos do ROS 2 para a função desejada e observe o recebimento de dados.
4. Observe por meio do Gazebo o movimento do robô ou a mudança de valores em seu componente Pose.

