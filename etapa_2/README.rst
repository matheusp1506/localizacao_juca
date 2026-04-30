Etapa 2
#######

.. contents::
   :local:
   :depth: 2


Visão geral
***********

A etapa 2 começa o desenvolvimento do projeto, com estudo das bibliotecas aplicadas no projeto original, acréscimos à elas e então o seu uso para um protótipo inicial de fusão dos sensores.


Desenvolvimento
***************

Para a construção do projeto, primeiro buscou-se analisar sobre as bibliotecas já utilizadas, como 'bdc-motor' e a versão já disponível para o MPU-6050, encontradas no projeto original do robô `Juca <https://github.com/MatheusPinto/Juca>`_, desenvolvido pelo professor `Matheus Pinto <https://github.com/MatheusPinto>`_ do Instituto Federal de Santa Catarina. O mesmo código do projeto original foi utilizado para o teste dos sensores, garantindo o funcionamento dos mesmos.

Além do que já se encontrava presente, funções extras eram necessárias, então vieram acréscimos.

Bibliotecas
===========

Com a ideia inicial encontrada na primeira etapa, um quaternion se faz necessário para os cálculos de posição e para o recebimento no ROS. Além de fórmulas para o cálculo, o interesse no uso do Digital Motion Processor (DMP) do MPU-6050 surge, para obter os quaternions diretamente do sensor.

Buscas sobre o MPU-6050 trazem detalhes sobre seu DMP, entretanto não foram encontradas informações suficientes em datasheets para a construção de uma biblioteca, porém, encontrou-se uma biblioteca desenvolvida por `Jeff Rowberg <https://github.com/jrowberg>`_ chamada de `I2Cdev library collection <https://github.com/jrowberg/i2cdevlib>`_, com as funções necessárias. Então os detalhes desta biblioteca foram portadas para a estrutura utilizada já no projeto.

Além dos sensores, a comunicação com o computador passa a necessitar da biblioteca padrão da Espressif para conexão com a internet e a comunicação via MQTT.


Testes das Bibliotecas
======================

O uso de bibliotecas novas, além de mudanças na biblioteca do MPU-6050, demanda novos testes que agora alcancem estas tarefas.

O teste inicial foi uma modificação do projeto original do robô Juca, com a task do IMU o envio de dados para uma página funcionando na placa para a visualização do quaternion.

Para a comunicação com o MQTT, foi primeiro feito a forma inicial de fusão dos sensores.

Estimativa de Posição
=====================

Testes de Estimativas e MQTT
============================

(Outras subseções se necessário)
================================


Referências (links/datasheets/livros)
*************************************

- `Juca <https://github.com/MatheusPinto/Juca>`_
- `I2C Device Library <https://github.com/jrowberg/i2cdevlib>`_
- `ESP-MQTT <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/protocols/mqtt.html>`_
- `Espressif: Wi-Fi <https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/network/esp_wifi.html>`_

