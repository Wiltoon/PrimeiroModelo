
#include <ilcplex/ilocplex.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <vector>

using std::fstream;
using std::ofstream;
using std::ifstream;
using std::ios;
using std::string;
using namespace std;
ILOSTLBEGIN


struct Pedido {
	int id;
	int x;
	int y;
	int demanda;
	int beginTime;
	int endTime;
	int serviceTime;
};

struct CVRP {
	IloNumArray p;						// Pedidos
	IloArray <IloBoolVarArray> x;		// Rotas atribuidas aos arcos (i,j)
	IloArray <IloNumArray> d;			// Distancia entre dois pontos
	IloNumVarArray u;					// Variavel auxiliar para eliminicao de rota
	IloNum Q;							// Capacidade maxima de qualquer veículo
};

struct SaidaEsperada {
	CVRP vars;
	IloModel modelo;
	IloEnv env;
};

int NUMERO_PEDIDOS = 20;
int CAPACIDADE_PESO_DOS_VEICULOS = 200;
int CUSTO_DE_VEICULO = 10;
int NUMERO_DE_VEICULOS = 25;

void readCsv(Pedido* pedidos);
void montarDistanciaTempoEntrePedidos(Pedido* pedidos, double** d, double** t);
double distanciaEuclidiana(Pedido origem, Pedido destino);

/*CVRP createParamsVars(IloEnv* env, Pedido* pedidos);

SaidaEsperada CVRPModel(IloEnv* env, IloModel* modelo, CVRP* vars);

void funcaoObjetivo(IloEnv *env, IloModel *modelo, CVRP* vars);

void criarRestricoes(IloEnv *env, IloModel *modelo, CVRP* vars);

void restricaoCapacidade(IloEnv *env, IloModel *modelo, CVRP* vars);
void restricaoDeChegada(IloEnv *env, IloModel *modelo, CVRP* vars);
void restricaoDeSaida(IloEnv *env, IloModel *modelo, CVRP* vars);
void restricaoEliminacaoDeSubRota(IloModel *modelo, CVRP* vars);
void restricaoComplementarDeEliminacao(IloEnv* env, IloModel *modelo, CVRP *vars);*/


int main(int argc, char** argv) {

	IloEnv env; //declara o ambiente e dá o nome de "env"
	try {
		int N = NUMERO_PEDIDOS;
		IloModel modelo(env); //declara o modelo e dá o nome de "modelo"

		Pedido* pedidos = (Pedido*)malloc(sizeof(Pedido) * NUMERO_PEDIDOS);
		readCsv(pedidos);
		// criacao de parametros e variaveis
		IloNumArray p(env, N);								// Peso requisitado por um cliente
		for (int i = 0; i < N; i++) {
			p[i] = pedidos[i].demanda;
		}
		IloNum Q = CAPACIDADE_PESO_DOS_VEICULOS;

		IloArray <IloBoolVarArray> x(env, N);				// Rotas atribuidas aos arcos (i,j)
		for (int i = 0; i < N; i++) {
			x[i] = IloBoolVarArray(env, N);
		}

		IloArray <IloNumArray> d(env, N);					// Matrizes de distancia e tempo entre pedido i ate j
		for (int i = 0; i < N; i++) {
			d[i] = IloNumArray(env, N);
		}
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				if (i == j) {
					d[i][j] = 1;
					x[i][j].setBounds(0, 0);
				}
				else {
					d[i][j] = distanciaEuclidiana(pedidos[i], pedidos[j]);
					x[i][j].setBounds(0, 1);
				}
				//cout << d[i][j] << " ";
			}
			//cout << endl;
		}

		IloNumVarArray u(env, N, 0, IloInfinity);							// Variavel auxiliar para eliminicao de rota

		CVRP vars;
		vars.d = d;
		vars.p = p;
		vars.Q = Q;
		vars.u = u;
		vars.x = x;

		//criacao do modelo
		//funcao objetivo 
		IloExpr fo(env);
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				fo += d[i][j] * x[i][j];
			}
		}
		modelo.add(IloMinimize(env, fo));
		fo.end();
		//restricoes
		//capacidade
		for (int i = 0; i < N; i++) {
			IloExpr restCapacity(env);
			for (int j = 0; j < N; j++) {
				if (i != j) {
					restCapacity += p[i] * x[i][j];
				}
			}
			modelo.add(restCapacity <= Q);
			restCapacity.end();
		}
		//deposito
		IloExpr depCli(env);
		IloExpr cliDep(env);
		int qntMinVei = 0;		
		for (int i = 1; i < N; i++) {
			depCli += x[0][i];
			cliDep += x[i][0];
			qntMinVei += p[i];
		}
		qntMinVei = qntMinVei / Q + 1;
		cout << "Armazenamento =" << qntMinVei << endl;
		modelo.add(depCli >= qntMinVei);
		//modelo.add(cliDep >= qntMinVei);
		modelo.add(depCli == cliDep);
		depCli.end();
		cliDep.end();
		//chegada e saida
		for (int j = 1; j < N; j++) {
			IloExpr restChegada(env);
			for (int i = 0; i < N; i++) {
				if (i != j) {
					restChegada += x[i][j];
				}
			}
			modelo.add(restChegada == 1);
			restChegada.end();
		}
		for (int i = 1; i < N; i++) {
			IloExpr restSaida(env);
			for (int j = 0; j < N; j++) {
				if (j != i) {
					restSaida += x[i][j];
				}
			}
			modelo.add(restSaida == 1);
			restSaida.end();
		}
		//eliminacao1
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				if (i != j) {
					modelo.add(u[i] >= u[j] + p[j] - Q * (1 - x[i][j]));
				}
			}
		}
		//eliminacao2
		for (int i = 1; i < N; i++) {
			modelo.add(u[i] >= p[i]);
			modelo.add(u[i] <= Q);
		}

		SaidaEsperada out;
		out.env = env;
		out.modelo = modelo;
		out.vars = vars;
		//

		IloArray<IloExtractableArray> relaxa(env, N);
		for (int i = 0; i < N; i++) {
			relaxa[i] = IloExtractableArray(env, N);
		}
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				if (i == j) {
					x[i][j].setBounds(0, 0);
				}
				else {
					x[i][j].setBounds(0, 1);
				}
				relaxa[i][j] = IloConversion(env, x[i][j], ILOFLOAT);
				modelo.add(relaxa[i][j]);
			}
		}

		IloCplex cplex(modelo);
		//cplex.setOut(env.getNullStream());
		IloNum objFO = IloInfinity;
		//Salvar solução
		IloArray <IloFloatArray> sol(env, N);
		for (int i = 0; i < N; i++) {
			sol[i] = IloFloatArray(env, N);
		}
		//Relax-and-Fix por períodos
		vector<int> visitar;
		vector<int> visitado;
		visitar.push_back(0);
		for (int t = 0; visitado.size() <= N; t++) {
			//cout << "TAMANHO ATUAL DOS VISITADOS = " << visitado.size() << endl; 
			vector<int> auxvisitar;
			cout << "VISITADO[";
			for (int i = 0; i < visitado.size(); i++) {
				cout << visitado[i] << ", ";
			}
			cout << "]" << endl;
			cout << "VISITAR[";
			for (int i = 0; i < visitar.size(); i++){
				cout << visitar[i] << ", ";
			}
			cout << "]" << endl;

			cout << "ITERADOR" << t << "\t" << objFO << endl;
			//Remove as relaxões que serão resolvidas em binário
			for (int i = 0; i < visitar.size(); i++) {
				for (int j = 0; j < N; j++) {
					modelo.remove(relaxa[visitar[i]][j]);
				}
			}
			cplex.setParam(IloCplex::TiLim, 50);
			IloBool result = cplex.solve();
			if (result) {
				objFO = cplex.getObjValue();
				//cout << "result = " << objFO << endl;
				// Salva a parte inteira da solução
				for (int i = 0; i < visitar.size(); i++) {
					cplex.getValues(x[visitar[i]], sol[visitar[i]]);
				}
				cout << "Partial Result:" << endl;
				for (int i = 0; i < N; i++) {
					for (int j = 0; j < N; j++) {
						if (sol[i][j] >= 0.8) {
							cout << i << "-->" << j << endl;
						}
					}
				}
				//cout << "extract 2" << endl;
				//Fixa a parte inteira da solução
				for (int check = 0; check < visitar.size(); check++) {
					for (int i = 0; i < N; i++) {
						//cout << "sol [" << check << "][" << i << ']' << "=" << sol[check][i] << endl;
						if (sol[visitar[check]][i] >= 0.8) {
							cout << "sol [" << visitar[check] << "][" << i << ']' << "=" << sol[visitar[check]][i] << endl;
							// FIXA VALOR DE X[i][j] ja resolvido
							for (int row = 0; row < N; row++) {
								if (row != visitar[check]) {
									x[row][i].setBounds(0, 0);
								}
								else { 
									x[visitar[check]][i].setBounds(1, 1);
									x[i][visitar[check]].setBounds(0, 0);
								}
							}
							if (i != 0) {
								auxvisitar.push_back(i);
							}
							int encontrado = 0;
							for (int it = 0; it < visitado.size(); it++) {
								//cout << "VISITADO[" << it << "] = \t" << visitado[it] << endl;
								if (visitado[it] == i) {
									encontrado = 1;
								}
							}
							if (encontrado == 0) {
								if (i != 0) {
									visitado.push_back(i);
								}
							}
						}
						else {
							x[check][i].setBounds(0, 0);
						}
					}
				}
				// Apagar a memoria do visitar e colocar o valor do auxvisitar
				visitar.clear();
				//cout << "SIZE -> " << auxvisitar.size() << endl;
				for (int i = 0; i < auxvisitar.size(); i++) {
					visitar.push_back(auxvisitar[i]);
					//cout << "AUXVISITAR[" << i << "] = \t" << auxvisitar[i] <<endl;
				}
			}
			else {
				//DESFIXAR VARIVEIS FIXADAS E RESOLVER NOVAMENTE

				for (int i = 0; i < visitado.size(); i++) {
					for (int j = 0; j < N; j++) {
						x[visitado[i]][j].setBounds(0, 1);
						x[j][visitado[i]].setBounds(0, 1);
					}
				}
			}
		}
		cout << "Resultado Final" << objFO << endl;

		//cplex.extract(modelo);
		//cplex.exportModel("saida.lp");

		
		//if(!cplex.solve()) {
		//	env.error() << "Não se pode resolver!" << endl;
		///	throw(-1);
		//}
		//env.out() << "Funcao Objetivo = " << cplex.getObjValue() << endl;
		//env.out() << "Tempo = " << cplex.getTime() << endl;
		//env.out() << "Natureza da Solucao: " << cplex.getStatus() << endl;
		//printf("\n\n\n");
	}
	catch(IloException& ex) {
		cerr << "ERRO: " << ex << endl;
	}
	catch(...) {
		cerr << "ERRO!" << endl;
	}
	env.end();
	system("PAUSE");	
	return 0;
}

void readCsv(Pedido* pedidos) {
	int endColumn = 7;
	int iterator = 0;
	int line = 0;
	string text;
	ifstream myfile("helper1.txt");
	if (myfile.is_open()) {
		while (getline(myfile, text, ',')) {
			//cout << text << ", ";
			switch (iterator) {
			case 0:
				pedidos[line].id = stoi(text);
				if (pedidos[line].id == NUMERO_PEDIDOS) {
					myfile.close();
					return;
				}
				//cout << "id=" << pedidos[line].id << endl;
				break;
			case 1:
				pedidos[line].x = stoi(text);
				//cout << "x=" << pedidos[line].x << endl;
				break;
			case 2:
				pedidos[line].y = stoi(text);
				//cout << "y=" << pedidos[line].y << endl;
				break;
			case 3:
				pedidos[line].demanda = stoi(text);
				//cout << "demanda=" << pedidos[line].demanda << endl;
				break;
			case 4:
				pedidos[line].beginTime = stoi(text);
				//cout << "BT=" << pedidos[line].beginTime << endl;
				break;
			case 5:
				pedidos[line].endTime = stoi(text);
				//cout << "ET=" << pedidos[line].endTime << endl;
				break;
			case 6:
				pedidos[line].serviceTime = stoi(text);
				//cout << "ST=" << pedidos[line].serviceTime << endl;
				break;
			default:
				break;
			}
			iterator++;
			if (iterator == endColumn) {
				line++;
				iterator = 0;
			}
		}
		myfile.close();
	}
}

void montarDistanciaTempoEntrePedidos(Pedido* pedidos, double** d, double** t) {
	for (int i = 0; i < NUMERO_PEDIDOS; i++) {
		for (int j = i; j < NUMERO_PEDIDOS; j++) {
			if (i == j) {
				d[i][j] = 1;
			}
			else if (i > j) {
				d[i][j] = IloInfinity;
			}
			else {
				d[i][j] = distanciaEuclidiana(pedidos[i], pedidos[j]);
			}
		}
	}
};
double distanciaEuclidiana(Pedido origem, Pedido destino) {
	double dx = (double)destino.x - (double)origem.x;
	double dy = (double)destino.y - (double)origem.y;
	return sqrt(dx * dx + dy * dy);
}

