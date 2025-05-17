# Computação Gráfica

## Atividade Vivencial 1
Nome: Marcelo Strack Daros

## Especificações:
* Para ler o .obj, a função loadOBJ foi implementada no Hello3D.cpp
* Foi utilizado um struct para armazenamento das propriedades de cada cubo, e vector para armazenamento de dois cubos
* Setas do teclado e teclas 1 e 2: rotações
* Teclas WASD e IJ: translações
* Teclas Z e X: escala
* Teclas Q e E: seleção dos cubos
* Inicialmente, o cubo da esquerda fica selecionado. Ao apertar a tecla E, o cubo da direita é selecionado, e ao apertar a tecla Q o cubo da esquerda é selecionado novamente. Para ficar mais dinâmico, fiz as operações de translação aplicadas apenas ao cubo selecionado, enquanto que rotações e escala são aplicadas em ambos cubos.