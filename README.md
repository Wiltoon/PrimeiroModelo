# PrimeiroModelo
<p align="center">Problema de Roteamento de Veículos em C++</p>
<h4 align="center">
  ⚠️ Projeto em construção ⚠️
</h4>

### Features
- [x] Modelo Matemático
- [x] Relaxação das variáveis
- [x] Construção do R&F
<p align="center">
  <a href="#Sobre">Sobre |</a>
  <a href="#otimizador---cplex">Otimizador - CPLEX |</a>
  <a href="#modelo-matemático">Modelo Matemático |</a>
  <a href="#relax-and-fix">Relax-and-Fix |</a>
  <a href="#construção-do-rf">Construção do R&F |</a>
  <a href="#contato">Contato</a>
</p>

# Sobre

Este é um projeto que servirá como base para construção de um projeto voltado para o Problema de Roteamento Dinâmico de Veículos. 
O objetivo principal é explorar a heurística do Relax-and-Fix

# Otimizador - CPLEX

Foi utilizado somente o Otimizador disponibilizado pela IBM, caso queira testar o código é necessário ter o software CPLEX instalado e configurar o ambiente do Visual Studio 2019
para poder executar.

# Modelo Matemático

O modelo matemático a ser seguido está representado a seguir:
### Função Objetivo
- <a href="https://www.codecogs.com/eqnedit.php?latex=\min&space;\sum_{k}^{K}(\sum_{(i,j)}^{A}&space;c_{ij}x^k_{ij})&space;&plus;&space;e_{k}v_k" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\min&space;\sum_{k}^{K}(\sum_{(i,j)}^{A}&space;c_{ij}x^k_{ij})&space;&plus;&space;e_{k}v_k" title="\min \sum_{k}^{K}(\sum_{(i,j)}^{A} c_{ij}x^k_{ij}) + e_{k}v_k" /></a>
### Restrições
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{k}^{K}&space;w^k_{j}&space;=&space;1,&space;\qquad\qquad\forall&space;j&space;\neq&space;0" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{k}^{K}&space;w^k_{j}&space;=&space;1,&space;\qquad\qquad\forall&space;j&space;\neq&space;0" title="\sum_{k}^{K} w^k_{j} = 1, \qquad\qquad\forall j \neq 0" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{j}^{N}w^k_j&space;\le&space;Nv_k,&space;\qquad\qquad&space;\forall&space;k" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{j}^{N}w^k_j&space;\le&space;Nv_k,&space;\qquad\qquad&space;\forall&space;k" title="\sum_{j}^{N}w^k_j \le Nv_k, \qquad\qquad \forall k" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=y_i&plus;\sum_{k}^{K}z^k_i&space;=&space;1,&space;\qquad\qquad&space;\forall&space;i\neq0" target="_blank"><img src="https://latex.codecogs.com/gif.latex?y_i&plus;\sum_{k}^{K}z^k_i&space;=&space;1,&space;\qquad\qquad&space;\forall&space;i\neq0" title="y_i+\sum_{k}^{K}z^k_i = 1, \qquad\qquad \forall i\neq0" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{i}^{N}z^k_i&space;\le&space;Nv_k,&space;\qquad\qquad&space;\forall&space;k" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{i}^{N}z^k_i&space;\le&space;Nv_k,&space;\qquad\qquad&space;\forall&space;k" title="\sum_{i}^{N}z^k_i \le Nv_k, \qquad\qquad \forall k" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{i}^{N}x^k_{ij}&space;=&space;w^k_j,&space;\qquad\qquad\forall&space;k,j\neq0" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{i}^{N}x^k_{ij}&space;=&space;w^k_j,&space;\qquad\qquad\forall&space;k,j\neq0" title="\sum_{i}^{N}x^k_{ij} = w^k_j, \qquad\qquad\forall k,j\neq0" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{j}^{N}x^k_{ij}&space;=&space;z^k_i,&space;\qquad\qquad\forall&space;k,i" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{j}^{N}x^k_{ij}&space;=&space;z^k_i,&space;\qquad\qquad\forall&space;k,i" title="\sum_{j}^{N}x^k_{ij} = z^k_i, \qquad\qquad\forall k,i" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{(i,j)}^{N\times&space;N}x^k_{ij}&space;\le&space;Nv_k,&space;\qquad\qquad\forall&space;k" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{(i,j)}^{N\times&space;N}x^k_{ij}&space;\le&space;Nv_k,&space;\qquad\qquad\forall&space;k" title="\sum_{(i,j)}^{N\times N}x^k_{ij} \le Nv_k, \qquad\qquad\forall k" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{k}^{K}z^k_i&space;=&space;\sum_{k}^{K}\sum_{i}^{N}&space;x^k_{0i}&space;=&space;\sum_{i=1}^{N}y_i&space;=&space;\sum_k^K&space;v_k&space;\ge&space;K_{MIN}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{k}^{K}z^k_i&space;=&space;\sum_{k}^{K}\sum_{i}^{N}&space;x^k_{0i}&space;=&space;\sum_{i=1}^{N}y_i&space;=&space;\sum_k^K&space;v_k&space;\ge&space;K_{MIN}" title="\sum_{k}^{K}z^k_i = \sum_{k}^{K}\sum_{i}^{N} x^k_{0i} = \sum_{i=1}^{N}y_i = \sum_k^K v_k \ge K_{MIN}" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{k}^{K}w^k_j&space;=&space;\sum_{k}^{K}\sum_{i}^{N}&space;x^k_{i0}&space;=&space;0" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{k}^{K}w^k_j&space;=&space;\sum_{k}^{K}\sum_{i}^{N}&space;x^k_{i0}&space;=&space;0" title="\sum_{k}^{K}w^k_j = \sum_{k}^{K}\sum_{i}^{N} x^k_{i0} = 0" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=u^k_i&space;\ge&space;u^k_j&space;&plus;&space;p_i&space;-Q(1-x^k_{ij}),&space;\qquad\qquad&space;\forall&space;k,i,j" target="_blank"><img src="https://latex.codecogs.com/gif.latex?u^k_i&space;\ge&space;u^k_j&space;&plus;&space;p_i&space;-Q(1-x^k_{ij}),&space;\qquad\qquad&space;\forall&space;k,i,j" title="u^k_i \ge u^k_j + p_i -Q(1-x^k_{ij}), \qquad\qquad \forall k,i,j" /></a> 

# Relax-and-Fix
A heurística Relax-and-Fix conhecida como R&F, tem como objetivo diminuir o tempo de execução de um algoritmo que possuí uma programação inteiramente misto. A heurística transforma todas as variáveis inteiras do modelo em variáveis reais (podendo assumir valores reais, e não somente inteiros). Além disso, a heurística inicialmente relaxa todas as variáveis inteiras, após isso estas variáveis são separadas em um horizonte de planejamento, das quais, a cada iteração uma parcela destas variáveis são transformadas em inteiras novamente e consequentemente o que se tem é um conjunto de variáveis inteiras mais fácil de resolver com um otimizador que utiliza Branch-and-Cut neste caso (CPLEX). No caso do Problema de Roteamento de Veículos a variável inteira que será particionada é a variável binária **x**. Portanto os passos que deverão ser seguidos neste caso será apresentado na próxima seção.

# Construção do R&F
Neste caso a ideia da heurística é particionar o problema através da variável **x** a qual indica o caminho de um vértice a outro, para exemplificar temos um exemplo a seguir com 11 clientes para serem visitados. 
- Primeiro todas as variáveis de **x** estarão relaxadas
- A primeira iteração a ser resolvida de forma binária é encontrar o primeiro destino partindo do depósito, ou seja, a relaxação da primeira linha da variável **x** voltará a se tornar binária e o restante continuará relaxado valores entre 0 e 1.
- Na segunda iteração em diante todos os **clientes** visitados estarão fixados em 0, pois não se deve visitar um cliente mais de uma única vez, e a relaxação dos clientes visitados na iteração anterior serão removidas e voltarão a ser binários, mantendo a relaxação nas variáveis restantes.
- No final do processo a ideia é que todos os clientes sejam atendidos.  
![relaxfix](https://user-images.githubusercontent.com/14852213/141658901-e20db051-2e37-4d5b-915b-fe9f8e2fa8db.gif)

# Contato

