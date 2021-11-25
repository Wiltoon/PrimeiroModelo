
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

int NUMERO_PEDIDOS = 11;
int TIME_MAX = 60;
int SOMADOR_TIME = 3;
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
				char* char_arr;
				string name("x[" + to_string(i) + "][" + to_string(j) + "]");
				char_arr = &name[0];
				x[i][j].setName(char_arr);
				if (i == j) {
					d[i][j] = 999999;
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
		//deposito
		//chegada e saida
		IloConstraintArray cons_array_chegada(env);
		for (int j = 1; j < N; j++) {
			IloExpr restChegada(env); 
			char* namevar;
			string name("chegadaNoCliente[" + to_string(j)+"]");
			namevar = &name[0];
			for (int i = 0; i < N; i++) {
				if (i != j) {
					restChegada += x[i][j];
				}
			}
			IloConstraint consRestChegada = (restChegada == 1);
			consRestChegada.setName(namevar);
			modelo.add(consRestChegada);
			cons_array_chegada.add(consRestChegada);
			restChegada.end();
		}
		IloConstraintArray cons_array_saida(env);
		for (int i = 1; i < N; i++) {
			IloExpr restSaida(env);
			char* namevar;
			string name("saidaDoCliente[" + to_string(i) + "]");
			namevar = &name[0];
			for (int j = 0; j < N; j++) {
				if (j != i) {
					restSaida += x[i][j];
				}
			}
			IloConstraint consRestSaida = (restSaida == 1);
			consRestSaida.setName(namevar);
			modelo.add(consRestSaida);
			cons_array_saida.add(consRestSaida);
			restSaida.end();
		}
		IloExpr depCli(env);
		IloExpr cliDep(env);
		IloConstraintArray cons_deposit(env);
		int qntMinVei = 0;		
		for (int i = 1; i < N; i++) {
			depCli += x[0][i];
			cliDep += x[i][0];
			qntMinVei += p[i];
		}
		qntMinVei = qntMinVei / Q + 1;
		//cout << "Armazenamento =" << qntMinVei << endl;
		IloConstraint depositToClient = (depCli >= qntMinVei); 
		depositToClient.setName("saida_Deposito");
		IloConstraint clienToDeposit = (cliDep >= qntMinVei);
		clienToDeposit.setName("retorno_Deposito");
		IloConstraint inOut = (cliDep == depCli);
		modelo.add(inOut);
		modelo.add(depositToClient);
		modelo.add(clienToDeposit);
		cons_deposit.add(depositToClient);
		cons_deposit.add(clienToDeposit);
		cons_deposit.add(inOut);
		//modelo.add(depCli == cliDep);
		depCli.end();
		cliDep.end();
		inOut.end();
		//eliminacao1
		IloConstraintArray cons_MTZ(env);
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				if (i != j) {
					IloConstraint mtz = (u[i] >= u[j] + p[i] - Q * (1 - x[i][j]));
					char* char_arr;
					string name("MTZ:GETOUT_" + to_string(i) + "__" + to_string(j));
					char_arr = &name[0];
					mtz.setName(char_arr);
					cons_MTZ.add(mtz);
					modelo.add(mtz);
				}
			}
		}
		//eliminacao2
		for (int i = 0; i < N; i++) {
			char* char_arr;
			string name("u[" + to_string(i) + "]");
			char_arr = &name[0];
			u[i].setName(char_arr);
			if (i == 0) {
				modelo.add(u[i] >= 0);
			}
			else {
				modelo.add(u[i] >= p[i]);
			}
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
		IloArray <IloNumArray> sol(env, N);
		for (int i = 0; i < N; i++) {
			sol[i] = IloNumArray(env, N);
		}

		IloNumArray uSol(env, N);
		//Relax-and-Fix por períodos
		bool PRIMEIRAITERACAO = true;
		bool LOOPINFINITO = false;
		vector<int> visitar;
		vector<int> visitado;
		vector<int> pedidos_realizados;
 		visitar.push_back(0);
		//visitado.push_back(0);
		int tempo = 9;
		for (int t = 0; pedidos_realizados.size() < N; t++) {
			//PRINTER 
			/*PRINT*/
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
			cout << "x = [" << endl;
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					printf("%.2lf \t", sol[i][j]);
				}
				cout << endl;
			}
			cout << "]" << endl;
			cout << "ITERADOR" << t << "\t" << objFO << endl;
			//Remove as relaxões que serão resolvidas em binário
			if (PRIMEIRAITERACAO) {
				for (int i = 0; i < visitar.size(); i++) {
					for (int j = 0; j < N; j++) {
						modelo.remove(relaxa[visitar[i]][j]);
					}
				}
			}
			cplex.setParam(IloCplex::TiLim, tempo);
			cplex.extract(modelo);
			if (!LOOPINFINITO) {
				char* outputer;
				string saida("saida_" + to_string(t)+ ".lp");
				outputer = &saida[0];
				cplex.exportModel(outputer);
			}
			
			IloBool result = cplex.solve();

			if (result) {
				objFO = cplex.getObjValue();
				//cout << "result = " << objFO << endl;
				// Salva a parte inteira da solução
				for (int i = 0; i < visitar.size(); i++) {
					cplex.getValues(x[visitar[i]], sol[visitar[i]]);
				}
				cplex.getValues(u, uSol);
				cout << "Partial Result:" << endl;
				for (int i = 0; i < N; i++) {
					for (int j = 0; j < N; j++) {
						if (sol[i][j] >= 0.8) {
							cout << i << "-->" << j << "\t\tdistance = " << d[i][j] << "\t\tu["<< i <<"] =" << uSol[i] << "\t\tu[" << j << "] =" << uSol[j] << endl;
						}
					}
				}
				
				//cout << "extract 2" << endl;
				//Fixa a parte inteira da solução
				for (int check = 0; check < visitar.size(); check++) {
					for (int i = 0; i < N; i++) {
						if (sol[visitar[check]][i] >= 0.8) {
							cout << "sol [" << visitar[check] << "][" << i << ']' << "=" << sol[visitar[check]][i] << endl;
							// FIXA VALOR DE X[i][j] ja resolvido TALVEZ ANALISAR QUANDO PASSA O DEPOSITO POR AQUI!
							x[visitar[check]][i].setBounds(1, 1);
							for (int row = 0; row < N; row++) {//PERCORRER A COLUNA
								if (row != visitar[check]) {
									x[row][i].setBounds(0, 0);
								}
							}
							for (int col = 0; col < N; col++) {//PERCORRENDO A LINHA
								if (col != i) {
									x[visitar[check]][col].setBounds(0, 0);
								}
							}
							x[i][visitar[check]].setBounds(0, 0);//ZERANDO O SIMETRICO
							if (i != 0) {
								auxvisitar.push_back(i);
							}
							bool encontrado = false;
							for (int it = 0; it < visitado.size(); it++) {
								if (visitado[it] == i) {
									encontrado = true;
								}
							}
							if (i == 0) {
								x[visitar[check]][i].setBounds(0, 0);
								for (int dest = 1; dest < visitar.size(); dest++) {
									x[visitar[check]][dest].setBounds(0, 1);
								}
							}
							if (!encontrado) {
								if (i != 0) {
									visitado.push_back(i);
								}
							}
						}
						//else { // CONDIÇAO PRA ESSE AQUI ACONTECER
						//	x[check][i].setBounds(0, 0);
						//}
					}
				}
				// Apagar a memoria do visitar e colocar o valor do auxvisitar
				visitar.clear();
				//cout << "SIZE -> " << auxvisitar.size() << endl;
				// RELAXAR TODO MUNDO NOVAMENTE E NA HORA DE REMOVER A RELAXAÇAO FIXAR CLIENTES VISITADOS EM 0
				for (int i = 0; i < cons_MTZ.getSize(); i++) {
					modelo.remove(cons_MTZ[i]);
				}
				// CASO NAO TENHA MAIS NINGUEM PARA VISITAR (AUXVISITAR == 0) (O AUXVISITAR VIRA O VISITAR)
				if (auxvisitar.size() == 0) {

					LOOPINFINITO = true;
					// FIXAR TODOS OS PEDIDOS VISITADOS EM 0 EXCETO OS DEPOSITOS
					PRIMEIRAITERACAO = false;
					for (int el = 0; el < N; el++) {
						for (int visitei = 0; visitei < visitado.size(); visitei++) {
							// EXCETO OS VISITADOS QUE DEVEM FICAR FIXOS EM 0 (j == 0)
							int elemento_visitado = visitado[visitei];
							//if (el != elemento_visitado) { 
							//	//int elemento_nao_visitado = el;
							//	//// NAS SAIDAS DOS DEPOSITOS FIXAR EM 0 OS VISITADOS E O RESTANTE INTEIRO
							//	//if (elemento_nao_visitado == 0) {
							//	//	x[0][0].setBounds(0, 0);
							//	//}
							//	//else {
							//	//	cout << "x[0][" << elemento_nao_visitado << "] = " << elemento_nao_visitado << "!!!!" << endl;
							//	//	x[0][elemento_nao_visitado].setBounds(0, 1); // TORNA INTEIRO SE NAO FOI VISITADO
							//	//}
							//}
							//else {
							//	for (int i = 0; i < N; i++) {
							//		x[i][elemento_visitado].setBounds(0, 0); // ZERANDO COLUNA
							//		x[elemento_visitado][i].setBounds(0, 0); // ZERANDO LINHA
							//		cout << "x["<< i << "][" << elemento_visitado << "] = " << x[i][elemento_visitado] << "!!!!" << endl;
							//	}
							//}
							// PASSA OS VISITADOS PARA OS REALIZADOS
							if (el == 0) { // DEVE ADICIONAR OS REALIZADOS SOMENTE UMA VEZ
								pedidos_realizados.push_back(elemento_visitado);
							}
						}
					}
					cout << "PEDIDOS REALIZADOS = [";
					for (int kk = 0; kk < pedidos_realizados.size(); kk++) {
						cout << pedidos_realizados[kk] << ", ";
					}
					cout << "]" << endl;
					// ZERA A LISTA DE VISITADOS
					visitado.clear();
					auxvisitar.push_back(0); // RETORNA AO DEPOSITO
				}
				for (int i = 0; i < auxvisitar.size(); i++) {
					if (auxvisitar[i] != 0) {
						visitar.push_back(auxvisitar[i]);
					}
					//cout << "AUXVISITAR[" << i << "] = \t" << auxvisitar[i] <<endl;
				}
			}
			else {
				if (tempo < TIME_MAX) {
					cout << "Aumentar tempo do solve " << tempo << "+" << SOMADOR_TIME << endl;
					tempo = tempo + SOMADOR_TIME;
				}
				//if (visitado.size() < N - 1) {
				LOOPINFINITO = true;

				/*modelo.remove*/
				for (int naovisitei = 1; naovisitei < N; naovisitei++) {
					bool notvisit = true;
					for (int i = 0; i < visitado.size(); i++) {
						if (visitado[i] == naovisitei) {
							notvisit = false;
							break;
						}
					}
					if (notvisit) {
						visitado.push_back(naovisitei);
						visitar.push_back(naovisitei);
					}
				}
				if (visitado.size() == (N-1)) {
					for (int i = 0; i < visitado.size(); i++) {
						pedidos_realizados.push_back(visitado[i]);
					}
				}
				/*
				else {
					LOOPINFINITO = true;
				}*/
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
		for (int j = 0; j < NUMERO_PEDIDOS; j++) {
			if (i == j) {
				d[i][j] = 9999;
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

