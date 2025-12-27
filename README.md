# archive-ESP32-Arduino-Zapper


## Este projeto não está terminado. Está disponível aqui, para quem quiser aproveitar alguma ideia ou continuá-lo. Está completamente liberado, para copiar, reproduzir, aprimorar. Bom divertimento! ##
# (English version bellow)#




PONTOS BÁSICOS

Máquina IoT de baixo custo, baixo consumo, modular, múltiplas interfaces, protocolos e aplicações, que funcione autônoma off-line e/ou on-line.  Primeira aplicação: máquina com múltiplas funções para medicina integrativa.

Núcleo do sistema deve ser modular e ter como foco atender projetos de IoT, indústria 4.0, automação geral (industrial, predial, processos), equipamentos de supervisão, telemetria e relacionados com o bem-estar e a medicina. Todas as áreas têm em comum (processos locais e/ou remotos de) acionamento, supervisão, monitoramento de interfaces digitais, analógicas ou de comunicação. 

O núcleo de sistema que seja independente do hardware (abstração).

Prioridade para a utilização de software e bibliotecas free e open-source, que quando não o forem, serão de própria autoria.

Hardware deve ser de baixo custo, altamente integrado e com uma comunidade de desenvolvedores madura (inteligência coletiva).

O software (desenvolvido em C++), inicialmente, utilizou a base de conhecimento da arquitetura arduino (hardware e firmware) e suas bibliotecas. No entanto, no meio do projeto, evoluiu para arquitetura ESP32, da Espressif (www.espressif.com) chip 32 bits, dual core e altamente integrado com wifi, bluetooth, criptografia, 4 MB de flash, etc. Um grande entrave para ser, este, o hardware definitivo, é que o firmware, núcleo do IDF, é um software fechado, não é open-source.

Ainda se utiliza a estrutura de software e algumas bibliotecas do arduino. O software de ESP32 tem um layer que permite essa otimização na portabilidade do software anterior, mas, como consequência, não permite utilizar todo potencial do ESP32 e de seu framework próprio (IDF – IoT Development Framework) cuja base é o FreeRTOS.

Na próxima alteração, o software deverá ser compatível com IDF.

Outra análise a ser feita, no futuro, é a possibilidade de uma versão em Python, também utilizando ESP32.

Utiliza uma HMI, display touch da Nextion 3.5’’ (480x320) (www.nextion.tech), com comunicação serial - TTL, que já tem várias primitivas de criação de elementos gráficos, reduzindo o tempo de desenvolvimento drasticamente, em contrapartida, o custo também é maior e a qualidade gráfica não é a melhor.   

SEGURANÇA

Para ter acesso à máquina, cada operador tem o seu Login e tem dois tipos de operador, o operador administrador que tem acesso a efetuar cadastros e outras operações de administração e operador da máquina que executa funções da máquina (inicia/termina processos).
Todas as operações efetuadas são registradas e podem ser consultadas pelo (s) administrador (es).

O firmware da máquina (ESP32) tem vários algoritmos de criptografia e segurança (AES-256, SHA-1, -256, -384, -512, RSA) que podem ser utilizados para criptografar arquivos (dados) e a memória flash da máquina. Também tem proteção contra leitura do software da máquina (pelo efuse).

No caso da aplicação para equipamento médico, as interfaces com conexão corpo humano são todas isoladas oticamente e alimentadas com baterias.

## This project is not finished. It is available here for anyone who wants to use some of the ideas or continue it. It is completely free to copy, reproduce, and improve. Have fun! ##

BASIC POINTS

Low-cost, low-power, modular IoT machine with multiple interfaces, protocols, and applications, operating autonomously offline and/or online. First application: a multi-functional machine for integrative medicine.

The system core must be modular and focused on serving IoT, Industry 4.0, general automation (industrial, building, processes), supervisory equipment, telemetry, and wellness and medicine-related projects. All areas have in common (local and/or remote processes of) actuation, supervision, and monitoring of digital, analog, or communication interfaces.

The system core must be independent of the hardware (abstraction).

Priority will be given to the use of free and open-source software and libraries; when not, they will be of our own authorship.

Hardware should be low-cost, highly integrated, and backed by a mature developer community (collective intelligence).

The software (developed in C++) initially used the knowledge base of the Arduino architecture (hardware and firmware) and its libraries. However, midway through the project, it evolved to the ESP32 architecture from Espressif (www.espressif.com), a 32-bit, dual-core chip highly integrated with Wi-Fi, Bluetooth, encryption, 4 MB of flash, etc. A major obstacle to making this the definitive hardware is that the firmware, the core of the IDF, is closed-source software and not open-source.

The software structure and some Arduino libraries are still used. The ESP32 software has a layer that allows this optimization in the portability of the previous software, but as a consequence, it does not allow utilizing the full potential of the ESP32 and its own framework (IDF – IoT Development Framework), which is based on FreeRTOS.

In the next update, the software should be compatible with IDF.

Another analysis to be done in the future is the possibility of a Python version, also using ESP32.

It uses an HMI, a 3.5’’ Nextion touch display (480x320) (www.nextion.tech), with serial - TTL communication, which already has several primitives for creating graphical elements, drastically reducing development time. On the other hand, the cost is higher and the graphical quality is not the best.

SECURITY

To access the machine, each operator has their Login and there are two types of operators: the administrator operator, who has access to perform registrations and other administrative operations, and the machine operator, who executes machine functions (starts/ends processes). All operations performed are recorded and can be reviewed by the administrator(s).

The machine firmware (ESP32) has several encryption and security algorithms (AES-256, SHA-1, -256, -384, -512, RSA) that can be used to encrypt files (data) and the machine's flash memory. It also has protection against reading the machine's software (via efuse).

In the case of medical equipment applications, all interfaces that connect with the human body are optically isolated and battery-powered.