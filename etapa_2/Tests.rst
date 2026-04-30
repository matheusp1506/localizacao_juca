Testes da Etapa 2
#################

Teste de bibliotecas dos sensores
*********************************

O teste dos sensores utiliza o código encontrado na pasta codes/libraries. Este código é uma modificação do projeto original do robô Juca, com a task do IMU conjunta com o envio de dados para uma página funcionando na placa para a visualização do quaternion.
Este código apresenta a adição de bibliotecas, a modificação da biblioteca do MPU-6050, a página na forma de index.html e os códigos para conexão à internet e o uso de um webserver para a visualização do quaternion.

Para utilizar o código, basta seguir os seguintes passos:
#. Preparar o ambiente de desenvolvimento para o ESP32, com o uso da ESP-IDF v5.3.1.
#. Clonar o repositório do projeto e acessar a pasta da etapa 2, etapa_2, e então acessar a pasta codes/libraries.
#. Configurar as credenciais de Wi-Fi no arquivo ``wifi_manager.c``.
#. Compilar o código utilizando o comando ``idf.py build``.
#. Realizar o flash do código para a placa utilizando o comando ``idf.py flash``.
#. Acessar a página de teste utilizando o IP da placa, que pode ser encontrado no monitor serial, e visualizar o modelo 3D orientado conforme o quaternion recebido do sensor.

Teste de estimativa de posição e comunicação MQTT
************************************************

O teste de ambas a estimativa de posição e a comunicação via MQTT, foi construido três tasks diferentes, uma responsável por ler os dados de ambos os sensores, outra responsável por calcular a odometria e a última responsável por enviar os dados para o broker MQTT, e temporariamente para a página de teste, para a visualização do resultado.
A estrutura utiliza queues para desbloquear as tasks seguintes.

O código possui a leitura de ambos os sensores utilizando apenas uma task, portanto a leitura é realizada primeiro com o quaternion, por conter dados nem sempre disponíveis, como forma de não perder contagens de pulso dos encoders.
A task de cálculo da odometria é realizada logo após a leitura dos sensores, utilizando os dados obtidos para o cálculo da posição do robô, e então a task de envio é desbloqueada, enviando os dados para o broker MQTT e para a página de teste.

Para utilizar o código, basta seguir os seguintes passos:
#. Preparar o ambiente de desenvolvimento para o ESP32, com o uso da ESP-IDF v5.3.1.
#. Clonar o repositório do projeto e acessar a pasta da etapa 2, etapa_2, e então acessar a pasta codes/position.
#. Configurar as credenciais de Wi-Fi no arquivo ``wifi_manager.c``.
#. Configurar as credenciais do broker MQTT no arquivo ``mqtt_manager.h``.
#. Compilar o código utilizando o comando ``idf.py build``.
#. Realizar o flash do código para a placa utilizando o comando ``idf.py flash``.
#. Acessar a página de teste utilizando o IP da placa, que pode ser encontrado no monitor serial, e visualizar o quatenion, além de verificar o envio dos dados para o broker MQTT, utilizando um cliente MQTT para se inscrever nos tópicos configurados e visualizar os dados recebidos, como a estimativa de posição.