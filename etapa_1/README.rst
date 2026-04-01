Etapa 1
#######

.. contents::
   :local:
   :depth: 2


Visão geral
***********

A etapa 1 se concentra como uma etapa de pesquisas, com o intuito de entender e selecionar a maneira de realizar as futuras etapas.


Desenvolvimento
***************

Apresentar o desenvolvimento da etapa contendo detalhes de implementação (se houver) de hardware e software. Adicionar pesqusisas realizadas bem como testes realizados.

Para a construção do projeto, primeiro deve-se entender o funcionamento dos sistemas utilizados, portanto pesquisas sobre o uso do ROS (Robot Operating System) foram realizadas, com tutoriais seguidos para a instalação em um computador próprio. Além da instalação inicial, houve estudos sobre a aplicação da biblioteca micro-ROS para permitir a comunicação com o microcontrolador.

Tambem foi necessário o estudo sobre a odometria do robo, ou seja, como será feita a estimativa de posição dele, e consequentemente quais os tipos de dados que a comunicação do ROS utiliza para transmitir.

Testes
======

Para garantir a viabilidade do projeto, buscou-se já nesta etapa instalar e testar o ROS, no momento utilizando somente códigos de demonstração do mesmo. Seguindo o tutorial de instalação do ROS 2, foi instalada a versão mais recente, Kilted Kaiju, e então utilizada as demos de talker e listener.

Modelo Cinemático e Estimativa de Posição 
================================

Como o Juca foi projetado com uma configuração de tração diferencial, a base da nossa estimativa de posição (odometria) segue o modelo cinemático clássico para esse tipo de configuração. A ideia é usar a variação do movimento das rodas, captada pelos encoders de quadratura, combinada com a leitura de angulo do giroscópio MPU6050.

.. figure:: modelo_tracao_diferencial.png
   :width: 450px

Aqui nós temos como definir uma equaçao recursiva onde a posiçao atual é a soma da posiçao anterior com a variaçao gerada após o Δt, que no caso é o período de amostragem::

   x(k) = x(k-1) + Δx(k)

A posiçao do carrinho pode ser separado em 3 variáveis diferentes, x e y representando os pontos no eixo 2D em que o robo irá percorrer e o θ representando o angulo do mesmo, no qual podemos representar de acordo com a formulaçao abaixo::

    x(k) = x(k-1) + Δs * cos(θ(k-1) + Δθ/2)
    y(k) = y(k-1) + Δs * sin(θ(k-1) + Δθ/2)
    θ(k) = θ(k-1) + Δθ

Tendo como ponto de referencia o momento inicial do carrinho.

O Δs representado no modelo é o movimento linear médio do carrinho::

   Δs(k) = ΔD(k) * [ sin(Δθ(k) / 2) / (Δθ(k) / 2) ]

Onde o ΔD é o comprimento do arco real, calculado pela média do deslocamento das rodas::

   ΔD(k) = 1/2  * (Δφe*r + Δφd*r) 

Nesta implementação, o valor de Δθ é obtido diretamente da IMU (MPU6050), o que minimiza erros acumulados por derrapagem das rodas.

(Outras subseções se necessário)
================================


Referências (links/datasheets/livros)
*************************************

- `ROS 2 <https://www.ros.org/>`_
- `micro-ROS <https://micro.ros.org/>`_
- `ROS 2 - Instalação <https://docs.ros.org/en/kilted/Installation/Ubuntu-Install-Debs.html>`_



