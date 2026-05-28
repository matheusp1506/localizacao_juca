Etapa 3
#######

.. contents::
   :local:
   :depth: 2


Visão geral
***********

A etapa 3 concentra-se em aprimorar a fusão de sensores para a obtenção de dados mais próximos da realidade e a conexão e configuração do ROS para a tarefa desejada.


Desenvolvimento
***************

Considerando os focos da 3ª etapa, o desenvolvimento foi dividido em duas frentes, sendo uma direcionada aos dados retirados dos sensores, e outra direcionada a conexão, no computador, entre o ROS e o Gazebo, configurando o mesmo para o funcionamento desejado.

Visualização com Gazebo
=======================

Considerando o teste de visualização realizado no RViz durante a etapa 1, o formato inicial da forma de tratar dados do MQTT, enviados pelo robô, e os tópicos do ROS, já constroem um formato de trabalho, entretanto, a necessidade de novas construções, como a implementação de um filtro, para uso com os dados, e a visualização desejada sendo realizada no `Gazebo <https://gazebosim.org/docs/latest/getstarted/>`_, levam a mudanças em sua construção e comunicação.

Para o uso do Gazebo, foi feito um script Python que inicializa o mesmo, já preparando o 'robô' que aparece no ambiente virtual, este que é definido por um arquivo `.urdf <https://wiki.ros.org/urdf>`_ contendo sua aparência, dados de colisão e o plugin utilizado para permitir a modificação de sua posição e ângulo conforme a mudança detectada pelos sensores, `OdometryPublisher <https://gazebosim.org/api/sim/8/classgz_1_1sim_1_1systems_1_1OdometryPublisher.html>`_.

Testes
======

Para garantir o funcionamento das construções realizadas no Gazebo, foi construído versões alternativas das transmissões de dados, utilizando dados gerados para testes, a instalação das partes necessárias aos testes e o seu uso, se encontra em `Testes <Tests.rst>`_

Referências (links/datasheets/livros)
*************************************

- `Gazebo <https://gazebosim.org/docs/latest/getstarted/>`_
- `.urdf <https://wiki.ros.org/urdf>`_
- `OdometryPublisher <https://gazebosim.org/api/sim/8/classgz_1_1sim_1_1systems_1_1OdometryPublisher.html>`_



