//
#include <ilcplex/ilocplex.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <cstring>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <string>

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

int NUMERO_PEDIDOS = 100;
int CAPACIDADE_PESO_DOS_VEICULOS = 200;
int CUSTO_DE_VEICULO = 10;
int NUMERO_DE_VEICULOS = 25;

void readCsv(Pedido* pedidos);
void montarDistanciaTempoEntrePedidos(Pedido* pedidos, double** d, double** t);
double distanciaEuclidiana(Pedido origem, Pedido destino);

CVRP createParamsVars(IloEnv* env, Pedido* pedidos);

SaidaEsperada CVRPModel(IloEnv* env, IloModel* modelo, CVRP* vars);

void funcaoObjetivo(IloEnv *env, IloModel *modelo, CVRP* vars);

void criarRestricoes(IloEnv *env, IloModel *modelo, CVRP* vars);

void restricaoCapacidade(IloEnv *env, IloModel *modelo, CVRP* vars);
void restricaoDeChegada(IloEnv *env, IloModel *modelo, CVRP* vars);
void restricaoDeSaida(IloEnv *env, IloModel *modelo, CVRP* vars);
void restricaoEliminacaoDeSubRota(IloModel *modelo, CVRP* vars);
void restricaoComplementarDeEliminacao(IloEnv* env, IloModel *modelo, CVRP *vars);


int main(int argc, char** argv) {

	IloEnv env; //declara o ambiente e dá o nome de "env"
	try {
		int N = NUMERO_PEDIDOS;
		IloModel modelo(env); //declara o modelo e dá o nome de "modelo"

		Pedido* pedidos = (Pedido*)malloc(sizeof(Pedido) * NUMERO_PEDIDOS);
		readCsv(pedidos);

		CVRP vars = createParamsVars(&env,pedidos);
		SaidaEsperada output = CVRPModel(&env, &modelo, &vars);
		IloArray<IloExtractableArray> relaxa(output.env, N);
		for (int i = 0; i < N; i++) {
			relaxa[i] = IloExtractableArray(output.env, N);
		}
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				relaxa[i][j] = IloConversion(output.env, output.vars.x[i][j], ILOFLOAT);
			}
		}

		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				output.modelo.add(relaxa[i][j]);
				if (i == j) {
					output.vars.x[i][j].setBounds(0, 0);
				}
				else {
					output.vars.x[i][j].setBounds(0, 1);
				}
			}
		}
		
		IloCplex cplex(output.modelo);
		cplex.setOut(output.env.getNullStream());
		IloNum objFO = IloInfinity;
		//Salvar solução
		IloArray <IloNumArray> sol(output.env, N);
		for (int i = 0; i < N; i++) {
			sol[i] = IloNumArray(output.env, N);
		}
		//Relax-and-Fix por períodos
		for (int t = 0; t < N; t++) {
			cout << "Iteração" << t << "\t" << objFO << endl;
			//Remove as relaxões do período
			for (int i = 0; i < N; i++) {
				output.modelo.remove(relaxa[i][t]);
			}
			cplex.solve();
			//cout << "It :" << t << endl;
			objFO = cplex.getObjValue();
			//cout << "result = " << objFO << endl;
			// Salva a parte inteira da solução
			//cout << "extract 1" << endl;
			for (int i = 0; i < N; i++) {
				cout << "vars = " << output.vars.x[t][i] << endl;
				cplex.getValue(output.vars.x[t][i], sol[t][i]);
			}
			//cout << "extract 2" << endl;
			//Fixa a parte inteira da solução
			for (int i = 0; i < N; i++) {
				if (sol[t][i] > 0) {
					cout << "sol [" << t << "][" << i << ']' << "=" << sol[t][i] << endl;
				}
				if (sol[t][i] >= 0.8) {
					cout << "sol [" << t << "][" << i << ']' << "=" << sol[t][i] << endl;
					output.vars.x[t][i].setBounds(1, 1);
				}
				else {
					output.vars.x[t][i].setBounds(0, 0);
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
	return dx * dx + dy * dy;
}

SaidaEsperada CVRPModel(IloEnv *env, IloModel *modelo, CVRP *vars) {
	funcaoObjetivo(&(*env), &(*modelo), &(*vars));
	criarRestricoes(&(*env), &(*modelo), &(*vars));
	SaidaEsperada out;
	out.env = *env;
	out.modelo = *modelo;
	out.vars = *vars;

	return out;
}
void criarRestricoes(IloEnv *env, IloModel *modelo, CVRP *vars) {
	restricaoCapacidade(&(*env), &(*modelo), &(*vars));
	restricaoDeChegada(&(*env), &(*modelo), &(*vars));
	restricaoDeSaida(&(*env), &(*modelo), &(*vars));
	restricaoEliminacaoDeSubRota(&(*modelo), &(*vars));
	restricaoComplementarDeEliminacao(&(*env), &(*modelo), &(*vars));
}

void restricaoCapacidade(IloEnv *env, IloModel *modelo, CVRP *vars) {
	//sum Pi*Xij <= Qk
	int N = NUMERO_PEDIDOS;
	for (int i = 0; i < N; i++) {
		IloExpr restCapacity(*env);
		for (int j = 0; j < N; j++) {
			restCapacity += (*vars).p[i] * (*vars).x[i][j];
		}
		(*modelo).add(restCapacity <= (*vars).Q);
		restCapacity.end();
	}
}
void restricaoDeChegada(IloEnv *env, IloModel *modelo, CVRP *vars) {
	//sum Xij = 1 i!=j
	int N = NUMERO_PEDIDOS;

	for (int j = 0; j < N; j++) {
		IloExpr restChegada(*env);
		for (int i = 0; i < N; i++) {
			if (i != j) {
				restChegada += (*vars).x[i][j];
			}
		}
		(*modelo).add(restChegada == 1);
		restChegada.end();
	}
}
void restricaoDeSaida(IloEnv *env, IloModel *modelo, CVRP* vars) {
	//
	int N = NUMERO_PEDIDOS;
	for (int i = 0; i < N; i++) {
		IloExpr restSaida(*env);
		for (int j = 0; j < N; j++) {
			if (j != i) {
				restSaida += (*vars).x[i][j];
			}
		}
		(*modelo).add(restSaida == 1);
		restSaida.end();
	}
}
void restricaoEliminacaoDeSubRota(IloModel *modelo, CVRP* vars) {
	// if Xij == 1 --> Ui + Qi == Uj 
	int N = NUMERO_PEDIDOS;
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (i != j) {
				(*modelo).add((*vars).u[j] >= (*vars).u[i] + (*vars).p[i] - (*vars).Q * (1 - (*vars).x[i][j]));
			}
		}
	}
}
void restricaoComplementarDeEliminacao(IloEnv *env, IloModel *modelo, CVRP* vars) {
	// q[i] <= u[i] <= Qmax
	int N = NUMERO_PEDIDOS;
	for (int i = 0; i < N; i++) {
		IloExpr urest(*env);
		urest = (*vars).u[i];
		(*modelo).add(urest >= (*vars).p[i]);
		(*modelo).add(urest <= (*vars).Q);
		urest.end();
	}
}

void funcaoObjetivo(IloEnv *env, IloModel *modelo, CVRP *vars) {
	int N = NUMERO_PEDIDOS;
	IloExpr fo(*env);
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			fo += (*vars).d[i][j] * (*vars).x[i][j];
		}
	}
	(*modelo).add(IloMinimize(*env, fo));
	fo.end();
}


CVRP createParamsVars(IloEnv *env, Pedido* pedidos) {
	int N = NUMERO_PEDIDOS;
	IloNumArray p(*env, N);								// Peso requisitado por um cliente
	for (int i = 0; i < N; i++) {
		p[i] = pedidos[i].demanda;
	}
	IloNum Q = CAPACIDADE_PESO_DOS_VEICULOS;

	IloArray <IloBoolVarArray> x(*env, N);				// Rotas atribuidas aos arcos (i,j)
	for (int i = 0; i < N; i++) {
		x[i] = IloBoolVarArray(*env, N);
	}

	IloArray <IloNumArray> d(*env, N);					// Matrizes de distancia e tempo entre pedido i ate j
	for (int i = 0; i < N; i++) {
		d[i] = IloNumArray(*env, N);
	}		
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			if (i == j) {
				d[i][j] = -1;
				x[i][j].setBounds(0, 0);
			}
			else {
				d[i][j] = distanciaEuclidiana(pedidos[i], pedidos[j]);
				x[i][j].setBounds(0, 1);
			}
		}
	}

	IloNumVarArray u(*env, N, 0, IloInfinity);							// Variavel auxiliar para eliminicao de rota

	CVRP vars;
	vars.d = d;
	vars.p = p;
	vars.Q = Q;
	vars.u = u;
	vars.x = x;
	return vars;
}