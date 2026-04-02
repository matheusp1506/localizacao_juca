Testes da Etapa 1
#################

Teste de visualização do RViz
*****************************

Para replicar os testes, os seguintes passos podem ser seguidos:

#. Instalar o ROS 2 Kilted Kaiju seguindo o tutorial oficial: `ROS 2 - Instalação <https://docs.ros.org/en/kilted/Installation/Ubuntu-Install-Debs.html>`_

#. Configurar um broker MQTT (como Mosquitto).

#. Utilizar o código ``fake_esp.py`` para simular o envio de dados do microcontrolador para o broker MQTT.

#. Instalar as dependências necessárias para o script ``ros_bridge.py``, como a biblioteca ``mqtt-client``, e usar o source do ROS usando os seguintes comandos:

   .. code-block:: bash

      source /opt/ros/kilted/setup.bash

   .. code-block:: bash

      pip install paho-mqtt

#. Utilizar o script ``ros_bridge.py`` para converter a comunicação MQTT para mensagens ROS.

#. Caso deseje apenas visualizar os dados recebidos, pode ser utilizado um echo dos topicos ROS:

   .. code-block:: bash

      ros2 topic echo /odom

#. Para uma visualização mais avançada, abrir o RViz:

   .. code-block:: bash

      rviz2

#. Inserir um display do tipo "Odometry" e configurar o tópico para "/odom" para visualizar a posição e orientação do robô.

    .. figure:: images/rviz_config.png
        :width: 450px

Após estes passos o resultado esperado deve ser algo semelhante a imagem abaixo, onde a odometria é apresentada pelas setas, indicando a posição e orientação do robô no espaço.

   .. figure:: images/rviz_arrows.png
      :width: 450px