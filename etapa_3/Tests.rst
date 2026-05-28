Testes da Etapa 3
#################

A seguir, encontra-se a forma de realizar os testes relacionados à etapa 3, feitos com o robô Juca.

Pré-requisitos
==============

Antes de iniciar os testes, certifique-se de que o ambiente de desenvolvimento possui as seguintes ferramentas e dependências configuradas:

* **ROS 2:** Instalado, na versão Kilted que está sendo utilizada, e com o *workspace* devidamente configurado (``source /opt/ros/...``, comando dependente da forma de instalação).
* **Simulador:** Gazebo instalado por meio da instalação ligada ao ROS 2, vista em: `Instalação <https://gazebosim.org/docs/latest/ros_installation/>`_.
* **Broker MQTT:** Um broker (como o Mosquitto) rodando localmente ou em rede para a comunicação dos scripts de ponte.
* **Dependências Python:** Certifique-se de ter instalado as bibliotecas a serem utilizadas, ``paho-mqtt``, ``plotly`` e ``pandas``.

Conjunto de Testes 1: Simulação e Movimentação no Gazebo via MQTT
=================================================================

**Objetivo:**
Validar a integração da ponte MQTT com o simulador Gazebo, garantindo que os comandos de telemetria/posição publicados via MQTT façam o robô se movimentar corretamente no ambiente simulado através de um *parameter bridge*.

Utilizar os arquivos encontrados na pasta `Gazebo <codes/gazebo>`_

Procedimento de Execução
------------------------

1.  Certifique-se de que o seu broker MQTT local está ativo e rodando.
2.  Em um terminal, inicie a simulação do robô Juca no Gazebo através do arquivo de inicialização do ROS 2:
    
    .. code-block:: bash

       ros2 launch juca_sim.launch.py

3.  Em um segundo terminal, execute o script da ponte MQTT para fazer a interface com os tópicos do ROS 2:
    
    .. code-block:: bash

       python3 mqtt_bridge.py

4.  Em um terceiro terminal, execute o script simulador do robô para começar a publicar dados falsos (*mock data*) de pose via MQTT ou utilize algum código anterior a esta etapa que faça este procedimento no robô, se atentando a estrutura utilizada:
    
    .. code-block:: bash

       python3 mock_robot.py

5.  Utilize a ferramenta de *parameter bridge* do ROS 2 para mapear o comando de alteração de pose (``set pose``):

    .. code-block:: bash

      ros2 run ros_gz_bridge parameter_bridge /world/empty/set_pose@ros_gz_interfaces/srv/SetEntityPose

Resultados Esperados
--------------------

* O Gazebo deve abrir exibindo o robô Juca e o cenário da simulação sem erros de carregamento de modelo.
* O terminal do ``mqtt_bridge.py`` deve exibir os logs de conexão com o broker e a recepção das mensagens vindas do ``mock_robot.py``.
* **Validação Visual:** O modelo do robô Juca deve se mover fisicamente na tela do Gazebo acompanhando as atualizações de pose enviadas.

Conjunto de Testes 2: Filtragem de Odometria com Filtro EKF
===========================================================

**Objetivo:**
Testar o comportamento do filtro de fusão sensorial Extended Kalman Filter (EKF) ao processar as mensagens da ponte MQTT e retransmitir a estimativa refinada de localização para o ambiente do Gazebo.

Utilizar os arquivos encontrados na pasta `Filtered <codes/filtered>`_

Procedimento de Execução
------------------------

1.  Mantenha o broker MQTT ativo e o simulador do Gazebo rodando (via ``juca_sim.launch.py``).
2.  Carregue o código na pasta `ESP <codes/filtered/esp>`_ no robô para obter os dados.
3.  Inicie o script responsável por receber os dados dos sensores via MQTT e aplicar o algoritmo de filtragem EKF:
    
    .. code-block:: bash

       python3 etapa_3/codes/mqtt_bridge_ekf.py

4.  Execute o nó ou script encarregado de pegar a saída tratada pelo filtro EKF e aplicá-la ao robô dentro do simulador:
    
    .. code-block:: bash

       python3 etapa_3/codes/ekf_to_gazebo.py

5.  Utilize a ferramenta de *parameter bridge* do ROS 2 para mapear o comando de alteração de pose (``set pose``):

    .. code-block:: bash

      ros2 run ros_gz_bridge parameter_bridge /world/empty/set_pose@ros_gz_interfaces/srv/SetEntityPose

6.  Execute o **robot_localization** usando o filtro definido:

    .. code-block:: bash

      ros2 run robot_localization ekf_node --ros-args --params-file ./ekf.yaml

Resultados Esperados
--------------------

* O script ``mqtt_bridge_ekf.py`` deve converter e repassar os dados ao robot_localization.
* O script ``ekf_to_gazebo.py`` deve mover os dados para o simulador.
* **Validação Visual:** O robô Juca deve se mover no Gazebo, conforme a resposta do filtro.


Conjunto de Testes 3: Visualização Gráfica da Trajetória (HTML)
===============================================================

**Objetivo:**
Validar a geração de uma página que apresente a trajetória realizada pelo robô.

Utilizar os arquivos encontrados na pasta `Log View <codes/log_view>`_

Procedimento de Execução
------------------------

1.  Certifique-se de possuir um arquivo de log ou base de dados populada com as coordenadas gravadas durante o teste 1 (Logs gerados por EKF ainda não são suportados).
2.  Execute o script de visualização apontando para os dados gerados:
    
    .. code-block:: bash

       python3 visualize.py

3.  Localize o arquivo de saída gerado pelo script (geralmente nomeado como ``juca_map.html`` ou similar na pasta do projeto).
4.  Abra o arquivo gerado em qualquer navegador web de sua preferência.

Resultados Esperados
--------------------

* O script deve executar até o fim sem apontar erros de manipulação de dados ou falta de bibliotecas.
* Um arquivo com extensão ``.html`` deve ser criado no diretório especificado.
* Ao abrir o navegador, deve ser exibido um gráfico que apresenta o caminho percorrido pelo robô Juca durante os testes.
