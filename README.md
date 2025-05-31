# Computação Gráfica

## Atividade Vivencial 2
Nome: Marcelo Strack Daros

## Especificações:
* Essa atividade foi desenvolvida com base no desafio do módulo 4 (já entregue), mas usando apenas um cubo.
* No Fragment Shader, foi criada a função CalcLight(), para calcular as reflexões e variáveis de cada um dos 3 pontos de luz.
* As variáveis lightDir e reflectDir foram removidas da main() do Fragment Shader e passadas para a CalcLight(). O mesmo foi feito com o fator de atenuação e com os coeficientes difuso e especular. Apenas a luz ambiente permaneceu na main, visto que essa não muda para nenhum ponto de luz.
* A struct PointLight foi criada para armazenar as informações de cada um dos 3 pontos de luz, e cada um foi criado como variável global.
* As teclas 4, 5 e 6 ligam/desligam cada ponto de luz:
    * Tecla 4: liga/desliga a luz principal
    * Tecla 5: liga/desliga a luz de preenchimento
    * Tecla 6: liga/desliga a luz de fundo
* No loop da aplicação, os pontos de luz são atualizados a cada frame.
* Eu já fiz a atividade do módulo 5 (sobre câmera), mas não implementei câmera nessa atividade. Portanto, para ver melhor o efeito da luz de fundo, é bom desligar as outras duas luzes e mover o cubo (via teclas WASD) para a esquerda ou direita. Assim o efeito da luz de fundo se torna mais visível, visto que a posição de visualização é estática. Ao rotacionar o cubo (via setas) a percepção das luzes também se torna mais evidente.