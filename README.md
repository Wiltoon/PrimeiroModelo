# PrimeiroModelo
<p align="center">Problema de Roteamento de Veículos em C++</p>
<h4 align="center">
  ⚠️ Projeto em construção ⚠️
</h4>

### Features
- [x] Modelo Matemático
- [x] Relaxação das variáveis
- [ ] Construção do R&F
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
- <a href="https://www.codecogs.com/eqnedit.php?latex=\min\sum\limits_{i&space;\in&space;V}\sum\limits_{j&space;\in&space;V}&space;x_{ij}d_{ij}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\min\sum\limits_{i&space;\in&space;V}\sum\limits_{j&space;\in&space;V}&space;x_{ij}d_{ij}" title="\min\sum\limits_{i \in V}\sum\limits_{j \in V} x_{ij}d_{ij}" /></a> 
### Restrições
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{i&space;\in&space;V'}x_{0i}&space;=&space;\sum_{i&space;\in&space;V'}x_{i0}&space;\ge&space;K_{MIN}" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{i&space;\in&space;V'}x_{0i}&space;=&space;\sum_{i&space;\in&space;V'}x_{i0}&space;\ge&space;K_{MIN}" title="\sum_{i \in V'}x_{0i} = \sum_{i \in V'}x_{i0} \ge K_{MIN}" /></a> 
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{j\in&space;V}x_{ij}&space;=&space;1&space;\quad&space;\forall&space;i\in&space;V',&space;i&space;\neq&space;j" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{j\in&space;V}x_{ij}&space;=&space;1&space;\quad&space;\forall&space;i\in&space;V',&space;i&space;\neq&space;j" title="\sum_{j\in V}x_{ij} = 1 \quad \forall i\in V', i \neq j" /></a> 
- <a href="https://www.codecogs.com/eqnedit.php?latex=\sum_{i\in&space;V}x_{ij}&space;=&space;1&space;\quad&space;\forall&space;j\in&space;V',&space;j&space;\neq&space;i" target="_blank"><img src="https://latex.codecogs.com/gif.latex?\sum_{i\in&space;V}x_{ij}&space;=&space;1&space;\quad&space;\forall&space;j\in&space;V',&space;j&space;\neq&space;i" title="\sum_{i\in V}x_{ij} = 1 \quad \forall j\in V', j \neq i" /></a>
- <a href="https://www.codecogs.com/eqnedit.php?latex=u_j&space;\ge&space;u_i&space;&plus;&space;p_j&space;-&space;Q_{max}(1&space;-&space;x_{ij})&space;\quad&space;\forall&space;i,j&space;\in&space;V,&space;i&space;\neq&space;j" target="_blank"><img src="https://latex.codecogs.com/gif.latex?u_j&space;\ge&space;u_i&space;&plus;&space;p_j&space;-&space;Q_{max}(1&space;-&space;x_{ij})&space;\quad&space;\forall&space;i,j&space;\in&space;V,&space;i&space;\neq&space;j" title="u_j \ge u_i + p_j - Q_{max}(1 - x_{ij}) \quad \forall i,j \in V, i \neq j" /></a> 
- <a href="https://www.codecogs.com/eqnedit.php?latex=p_i&space;\le&space;u_i&space;\le&space;Q_{max}\quad&space;\forall&space;i&space;\in&space;V'" target="_blank"><img src="https://latex.codecogs.com/gif.latex?p_i&space;\le&space;u_i&space;\le&space;Q_{max}\quad&space;\forall&space;i&space;\in&space;V'" title="p_i \le u_i \le Q_{max}\quad \forall i \in V'" /></a> 

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

