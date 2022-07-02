
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

int NUMERO_PEDIDOS = 101;
int TIME_MAX = 60;
int SOMADOR_TIME = 3;
int CAPACIDADE_PESO_DOS_VEICULOS = 400;
int CUSTO_DE_VEICULO = 10;
int NUMERO_DE_VEICULOS = 12;

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
		int K = NUMERO_DE_VEICULOS;
		IloModel modelo(env); //declara o modelo e dá o nome de "modelo"

		Pedido* pedidos = (Pedido*)malloc(sizeof(Pedido) * NUMERO_PEDIDOS);
		readCsv(pedidos);
		// criacao de parametros e variaveis
		IloNumArray p(env, N);								// Peso requisitado por um cliente
		for (int i = 0; i < N; i++) {
			p[i] = pedidos[i].demanda;
		}
		IloNum Q = CAPACIDADE_PESO_DOS_VEICULOS;
		IloNumArray e(env, K);
		e[0] = 30;
		e[1] = 10;
		e[2] = 20;
		e[3] = 30;
		e[4] = 10;
		e[5] = 20;
		e[6] = 30;
		e[7] = 10;
		e[8] = 20;
		e[9] = 30;
		e[10] = 10;
		e[11] = 20;

		IloArray <IloArray <IloBoolVarArray>> x(env, K);
		for (int k = 0; k < K; k++) {
			x[k] = IloArray <IloBoolVarArray>(env, N);				// Rotas atribuidas aos arcos (i,j)
			for (int i = 0; i < N; i++) {
				x[k][i] = IloBoolVarArray(env, N);
			}
		}
		IloBoolVarArray v(env, K);
		IloArray <IloNumArray> d(env, N);					// Matrizes de distancia e tempo entre pedido i ate j
		for (int i = 0; i < N; i++) {
			d[i] = IloNumArray(env, N);
		}
		IloBoolVarArray y(env, N);
		IloArray<IloBoolVarArray> w(env, K); // DESTINOS (index j)
		for (int k = 0; k < K; k++) {
			w[k] = IloBoolVarArray(env, N);
		}

		IloArray<IloBoolVarArray> z(env, K); // ORIGENS (index i)
		for (int k = 0; k < K; k++) {
			z[k] = IloBoolVarArray(env, N);
		}

		IloArray<IloNumVarArray> u(env, K);
		for (int k = 0; k < K; k++) {
			u[k] = IloNumVarArray(env, N, 0, CAPACIDADE_PESO_DOS_VEICULOS);							// Variavel auxiliar para eliminicao de rota
		}

		for (int k = 0; k < K; k++) {
			for (int i = 0; i < N; i++) {
				char* char_z;
				string name("z_" + to_string(k) + "_" + to_string(i));
				char_z = &name[0];
				z[k][i].setName(char_z);
				for (int j = 0; j < N; j++) {
					char* char_arr;
					string name("x_" + to_string(k) + "_" + to_string(i) + "_" + to_string(j));
					char_arr = &name[0];
					x[k][i][j].setName(char_arr);
					if (i == 0) {
						char* char_w;
						string name("w_" + to_string(k) + "_" + to_string(j));
						char_w = &name[0];
						w[k][j].setName(char_w);
					}
					if (i == j) {
						d[i][j] = 999999;
						x[k][i][j].setBounds(0, 0);
					}
					else {
						d[i][j] = distanciaEuclidiana(pedidos[i], pedidos[j]);
						x[k][i][j].setBounds(0, 1);
					}
					//cout << d[i][j] << " ";
				}
				//cout << endl;
			}
		}

		//criacao do modelo
		//funcao objetivo 
		IloExpr fo(env);
		for (int k = 0; k < K; k++) {
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					fo += d[i][j] * x[k][i][j] + e[k]*v[k];
				}
			}
		}
		modelo.add(IloMinimize(env, fo));
		fo.end();
		//restricoes
		//deposito
		//chegada e saida
		IloConstraintArray cons_destino(env);
		for (int j = 1; j < N; j++) {
			IloExpr restDest(env);
			char* namevar;
			string name("alguemChegaAoCliente[" + to_string(j) + "]");
			namevar = &name[0];
			for (int k = 0; k < K; k++) {
				restDest += w[k][j];
			}
			IloConstraint consRestDest = (restDest == 1);
			consRestDest.setName(namevar);
			modelo.add(consRestDest);
			cons_destino.add(consRestDest);
			restDest.end();
		}
		IloConstraintArray cons_destino_veiculo(env);
		for (int k = 0; k < K; k++) {
			IloExpr restDestDriver(env);
			char* namevarDD;
			string name("veiculo_dest_" + to_string(k));
			namevarDD = &name[0];
			for (int i = 0; i < N; i++) {
				restDestDriver += w[k][i];
			}
			IloConstraint consRestOriginDriver = (restDestDriver <= v[k] * N);
			consRestOriginDriver.setName(namevarDD);
			modelo.add(consRestOriginDriver);
			cons_destino_veiculo.add(consRestOriginDriver);
			restDestDriver.end();
		}

		IloConstraintArray cons_chegada(env);
		for (int i = 1; i < N; i++) {
			IloExpr restOrig(env);
			char* namevar;
			string name("alguemSaiDoCliente[" + to_string(i) + "]");
			namevar = &name[0];
			for (int k = 0; k < K; k++) {
				restOrig += z[k][i];
			}
			IloConstraint consRestOrigin = (restOrig + y[i] == 1);
			consRestOrigin.setName(namevar);
			modelo.add(consRestOrigin);
			cons_chegada.add(consRestOrigin);
			restOrig.end();
		}
		IloConstraintArray cons_chegada_veiculo(env);
		for (int k = 0; k < K; k++) {
			IloExpr restOrigDriver(env);
			char* namevarD;
			string name("veiculo_origin_" + to_string(k));
			namevarD = &name[0];
			for (int i = 0; i < N; i++) {
				restOrigDriver += z[k][i];
			}
			IloConstraint consRestOriginDriver = (restOrigDriver <= v[k] * N);
			consRestOriginDriver.setName(namevarD);
			modelo.add(consRestOriginDriver);
			cons_chegada.add(consRestOriginDriver);
			restOrigDriver.end();
		}

		IloConstraintArray cons_array_chegada(env);
		for (int k = 0; k < K; k++) {
			for (int j = 1; j < N; j++) {
				IloExpr restChegada(env); 
				char* namevar;
				string name("chegadaNoCliente_" + to_string(j) + "_peloVehicle_" + to_string(k));
				namevar = &name[0];
				for (int i = 0; i < N; i++) {
					if (i != j) {
						restChegada += x[k][i][j];
					}
				}
				IloConstraint consRestChegada = (restChegada == w[k][j]);
				consRestChegada.setName(namevar);
				modelo.add(consRestChegada);
				cons_array_chegada.add(consRestChegada);
				restChegada.end();
			}
		}
		IloConstraintArray cons_array_saida(env);
		for (int k = 0; k < K; k++) {
			for (int i = 0; i < N; i++) {
				IloExpr restSaida(env);
				char* namevar;
				string name("saidaDoCliente_" + to_string(i) + "_peloVehicle_" + to_string(k));
				namevar = &name[0];
				for (int j = 0; j < N; j++) {
					if (j != i) {
						restSaida += x[k][i][j];
					}
				}
				IloConstraint consRestSaida = (restSaida == z[k][i]);
				consRestSaida.setName(namevar);
				modelo.add(consRestSaida);
				cons_array_saida.add(consRestSaida);
				restSaida.end();
			}
		}

		IloConstraintArray cons_array_veiculos3(env);
		for (int k = 0; k < K; k++) {
			IloExpr restVeiX(env);
			char* namevar;
			string name("veiculo_" + to_string(k) + "_utilizado");
			namevar = &name[0];
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					restVeiX += x[k][i][j];
				}
			}
			IloConstraint consRestVei = (restVeiX <= v[k]*N);
			consRestVei.setName(namevar);
			modelo.add(consRestVei);
			cons_array_veiculos3.add(consRestVei);
			restVeiX.end();
		}
		IloExpr depCli(env);
		IloExpr depClient(env);
		IloExpr cliDep(env);
		IloExpr clientDep(env);
		IloConstraintArray cons_deposit(env);
		int qntMinVei = 0;	
		for (int k = 0; k < K; k++) {
			clientDep += w[k][0];
			depClient += z[k][0];
			for (int i = 1; i < N; i++) {
				depCli += x[k][0][i];
				cliDep += x[k][i][0];
			}
		}
		for (int i = 1; i < N; i++) {
			qntMinVei += p[i];
		}
		qntMinVei = qntMinVei / Q + 1;
		//cout << "Armazenamento =" << qntMinVei << endl;
		IloConstraint depositToClient = (depCli == qntMinVei);
		IloConstraint depositToClient2 = (depClient == qntMinVei);
		depositToClient.setName("saida_Deposito");
		depositToClient2.setName("saida_Deposito2");
		IloConstraint clienToDeposit = (cliDep == 0);
		IloConstraint clienToDeposit2 = (clientDep == 0);
		clienToDeposit.setName("retorno_Deposito");
		clienToDeposit2.setName("retorno_Deposito2");
		/*IloConstraint inOut = (cliDep == depCli);
		cons_deposit.add(inOut);
		inOut.end();
		modelo.add(inOut);*/
		modelo.add(depositToClient);
		modelo.add(clienToDeposit);
		cons_deposit.add(depositToClient);
		cons_deposit.add(clienToDeposit);
		//modelo.add(depCli == cliDep);
		depCli.end();
		cliDep.end();
		
		IloExpr sumVeiculos(env);
		for (int k = 0; k < K; k++) {
			char* namevarV;
			string nameV("v_" + to_string(k));
			namevarV = &nameV[0];
			v[k].setName(namevarV);
			sumVeiculos += v[k];
		}
		IloConstraint sumDrivers = (sumVeiculos >= qntMinVei);
		sumDrivers.setName("SumDrivers");
		modelo.add(sumDrivers);
		sumVeiculos.end();

		y[0].setBounds(0, 0);
		y[0].setName("y_0");
		IloExpr sumY(env);
		for (int j = 1; j < N; j++) {
			char* namevar;
			string name("y_" + to_string(j));
			namevar = &name[0];
			y[j].setName(namevar);
			sumY += y[j];
		}
		IloConstraint lastNode = (sumY == qntMinVei);
		lastNode.setName("UltimoNo");
		modelo.add(lastNode);
		sumY.end();
		//eliminacao1
		IloConstraintArray cons_MTZ(env);
		for (int k = 0; k < K; k++) {
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					if (i != j) {
						IloConstraint mtz = (u[k][i] >= u[k][j] + p[i] - Q * (1 - x[k][i][j]));
						char* char_arr;
						string name("GETOUT_" + to_string(k) + "_"+ to_string(i) +"_" + to_string(j));
						char_arr = &name[0];
						mtz.setName(char_arr);
						cons_MTZ.add(mtz);
						modelo.add(mtz);
					}
				}
			}
		}
		///eliminacao2
		for (int k = 0; k < K; k++) {
			for (int i = 0; i < N; i++) {
				char* char_arr;
				string name("u_" + to_string(k) + "_" + to_string(i));
				char_arr = &name[0];
				u[k][i].setName(char_arr);
				if (i == 0) {
					modelo.add(u[k][i] >= 0);
				}
				else {
					modelo.add(u[k][i] >= p[i]);
				}
				modelo.add(u[k][i] <= Q);
			}
		}

		IloArray <IloArray<IloExtractableArray>> relaxa(env, K);
		for (int k = 0; k < K; k++) {
			relaxa[k] = IloArray<IloExtractableArray>(env, N);
			for (int i = 0; i < N; i++) {
				relaxa[k][i] = IloExtractableArray(env, N);
			}
		}
		for (int k = 0; k < K; k++) {
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					if (i == j) {
						x[k][i][j].setBounds(0, 0);
					}
					else {
						x[k][i][j].setBounds(0, 1);
					}
					relaxa[k][i][j] = IloConversion(env, x[k][i][j], ILOFLOAT);
					modelo.add(relaxa[k][i][j]);
				}
			}
		}

		IloCplex cplex(modelo);
		//cplex.setOut(env.getNullStream());
		IloNum objFO = IloInfinity;
		//Salvar solução
		IloArray <IloArray <IloNumArray>> sol(env, K);
		for (int k = 0; k < K; k++) {
			sol[k] = IloArray <IloNumArray>(env, N);
			for (int i = 0; i < N; i++) {
				sol[k][i] = IloNumArray(env, N);
			}
		}

		IloArray<IloNumArray> uSol(env, K);
		for (int k = 0; k < K; k++) {
			uSol[k] = IloNumArray(env, N);
		}
		//Relax-and-Fix por períodos
		bool PRIMEIRAITERACAO = true;
		bool LOOPINFINITO = false;
		vector<int> visitar;
		vector<int> visitado;
		vector<int> pedidos_realizados;
 		visitar.push_back(0);
		//visitado.push_back(0);
		int tempo = 9;
		for (int t = 0; t < N/qntMinVei+1; t++) {
			//cout << "TAMANHO ATUAL DOS VISITADOS = " << visitado.size() << endl; 
			vector<int> auxvisitar;
			/*cout << "VISITADO[";
			for (int i = 0; i < visitado.size(); i++) {
				cout << visitado[i] << ", ";
			}
			cout << "]" << endl;
			*/
			cout << "VISITAR[";
			for (int i = 0; i < visitar.size(); i++){
				cout << visitar[i] << ", ";
			}
			cout << "]" << endl;
			/*cout << "x = [" << endl;
			for (int i = 0; i < N; i++) {
				for (int j = 0; j < N; j++) {
					printf("%.2lf \t", sol[i][j]);
				}
				cout << endl;
			cout << "]" << endl;
			}*/
			cout << "ITERADOR" << t << "\t" << objFO << endl;
			//Remove as relaxões que serão resolvidas em binário
			if (PRIMEIRAITERACAO) {
				for (int k = 0; k < K; k++) {
					for (int i = 0; i < visitar.size(); i++) {
						for (int j = 0; j < N; j++) {
							modelo.remove(relaxa[k][visitar[i]][j]);
						}
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
				for (int k = 0; k < K; k++) {
					cplex.getValues(u[k], uSol[k]);
					for (int i = 0; i < visitar.size(); i++) {
						cplex.getValues(x[k][visitar[i]], sol[k][visitar[i]]);
					}
				}
				//cplex.getValues(u, uSol);
				cout << "Partial Result:" << endl;
				for (int k = 0; k < K; k++) {
					for (int i = 0; i < N; i++) {
						for (int j = 0; j < N; j++) {
							if (sol[k][i][j] >= 0.8) {
								cout << k << ">: " << i << "-->" << j << "\t\tdistance = " << d[i][j] << "\t\tu[" << i << "] =" << uSol[k][i] << "\t\tu[" << j << "] =" << uSol[k][j] << endl; //<< "\t\tu[" << i << "] =" << uSol[i] << "\t\tu[" << j << "] =" << uSol[j]
							}
						}
					}
				}
				
				//cout << "extract 2" << endl;
				//Fixa a parte inteira da solução
				for (int check = 0; check < visitar.size(); check++) {
					for (int k = 0; k < K; k++) {
						for (int i = 1; i < N; i++) {
							//cout << "sol [" << check << "][" << i << ']' << "=" << sol[check][i] << endl;
							if (sol[k][visitar[check]][i] >= 0.8) {
								cout << "sol [" << visitar[check] << "][" << i << ']' << "=" << sol[k][visitar[check]][i] << endl;
								// FIXA VALOR DE X[i][j] ja resolvido
								x[k][visitar[check]][i].setBounds(1, 1);
								w[k][i].setBounds(1, 1);
								z[k][visitar[check]].setBounds(1, 1);
								y[i].setBounds(0, 0);
								int entrega = i;
								for (int veiculo = 0; veiculo < K; veiculo++) {
									char* namevarD;
									string name("fluxo_veiculo_" + to_string(veiculo) + "_da_entrega_" + to_string(entrega));
									namevarD = &name[0];
									IloConstraint consVeiculoUnico = (z[veiculo][entrega] + y[entrega] == w[veiculo][entrega]);
									consVeiculoUnico.setName(namevarD);
									modelo.add(consVeiculoUnico);
								}
								for (int row = 0; row < N; row++) {	//PERCORRER A COLUNA
									if (row != visitar[check]) {
										x[k][row][i].setBounds(0, 0);
										for (int kar = 0; kar < K; kar++) { // INUTILIZAR VEICULOS PARA ATENDER OS PEDIDOS DA COLUNA
											if (sol[kar][visitar[check]][i] >= 0.8) {
												x[kar][visitar[check]][i].setBounds(1, 1);
											}
											else {
												if (k != kar) {
													x[kar][visitar[check]][i].setBounds(0, 0);
												}
											}
										}
									}
								}
								for (int col = 0; col < N; col++) {//PERCORRENDO A LINHA
									if (col != i) {
										x[k][visitar[check]][col].setBounds(0, 0);
										for (int kar = 0; kar < K; kar++) { // INUTILIZAR VEICULOS PARA ATENDER OS PEDIDOS DA LINHA
											if (sol[kar][visitar[check]][col] >= 0.8) {
												x[kar][visitar[check]][col].setBounds(1, 1);
											}
											else {
												if (k != kar) {
													x[kar][visitar[check]][col].setBounds(0, 0);
												}
											}
										}
									}
								}
								for (int kar = 0; kar < K; kar++) { // INUTILIZAR VEICULOS PARA ATENDER OS PEDIDOS DOS SIMETRICOS
									x[kar][i][visitar[check]].setBounds(0, 0);
								}
								if (i != 0) {
									auxvisitar.push_back(i);
								}
								bool encontrado = false;
								for (int it = 0; it < visitado.size(); it++) {
									//cout << "VISITADO[" << it << "] = \t" << visitado[it] << endl;
									if (visitado[it] == i) {
										encontrado = true;
									}
								}
								if (!encontrado) {
									if (i != 0) {
										visitado.push_back(i);
									}
								}
							}
						}
					}
				}
				// Apagar a memoria do visitar e colocar o valor do auxvisitar
				visitar.clear();
				// CASO NAO TENHA MAIS NINGUEM PARA VISITAR (AUXVISITAR == 0) (O AUXVISITAR VIRA O VISITAR)
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
				LOOPINFINITO = true;

			}
		}
		cout << "Resultado Final" << objFO << endl;
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

