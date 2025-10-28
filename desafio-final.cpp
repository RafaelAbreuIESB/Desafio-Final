#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <omp.h>
#include <iomanip>
#include <chrono>
#include <algorithm>

using namespace std;

// Estrutura para armazenar uma pessoa
struct Pessoa {
    double estatura;
    double peso;
};

// Funcoes auxiliares
double calcularMedia(const vector<double>& pontos, const vector<int>& freq) {
    double soma = 0.0;
    int total = 0;
    #pragma omp parallel for reduction(+:soma,total)
    for (int i = 0; i < (int)pontos.size(); i++) {
        soma += pontos[i] * freq[i];
        total += freq[i];
    }
    return soma / total;
}

double calcularDesvioPadrao(const vector<double>& pontos, const vector<int>& freq, double media) {
    double soma = 0.0;
    int total = 0;
    #pragma omp parallel for reduction(+:soma,total)
    for (int i = 0; i < (int)pontos.size(); i++) {
        double dif = pontos[i] - media;
        soma += freq[i] * pow(dif, 2);
        total += freq[i];
    }
    return sqrt(soma / total);
}

// Funcao para gerar intervalos e frequencias
void gerarClasses(const vector<double>& dados, double intervalo, vector<double>& pontosMedios, vector<int>& freq) {
    if (dados.empty()) return;
    double minVal = *min_element(dados.begin(), dados.end());
    double maxVal = *max_element(dados.begin(), dados.end());

    int numClasses = ceil((maxVal - minVal) / intervalo);
    pontosMedios.resize(numClasses);
    freq.assign(numClasses, 0);

    // Calculo do ponto medio de cada classe
    for (int i = 0; i < numClasses; i++) {
        double inicio = minVal + i * intervalo;
        double fim = inicio + intervalo;
        pontosMedios[i] = (inicio + fim) / 2.0;
    }

    // Contagem de frequencias em paralelo
    #pragma omp parallel for
    for (int i = 0; i < (int)dados.size(); i++) {
        int classe = (int)((dados[i] - minVal) / intervalo);
        if (classe >= numClasses) classe = numClasses - 1;
        #pragma omp atomic
        freq[classe]++;
    }
}

int main() {
    vector<Pessoa> pessoas;
    ifstream arquivo("dados.txt");

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo de dados.txt" << endl;
        return 1;
    }

    double est, pes;
    while (arquivo >> est >> pes) {
        pessoas.push_back({est, pes});
    }
    arquivo.close();

    if (pessoas.empty()) {
        cout << "Nenhum dado encontrado no arquivo." << endl;
        return 1;
    }

    vector<double> estaturas, pesos;
    for (auto& p : pessoas) {
        estaturas.push_back(p.estatura);
        pesos.push_back(p.peso);
    }

    vector<double> pontosEst, pontosPes;
    vector<int> freqEst, freqPes;

    // Calcular tempo sequencial
    auto inicioSeq = chrono::high_resolution_clock::now();
    gerarClasses(estaturas, 8.0, pontosEst, freqEst);
    gerarClasses(pesos, 4.0, pontosPes, freqPes);

    double mediaEstSeq = calcularMedia(pontosEst, freqEst);
    double desvioEstSeq = calcularDesvioPadrao(pontosEst, freqEst, mediaEstSeq);
    double cvEstSeq = (desvioEstSeq / mediaEstSeq) * 100.0;

    double mediaPesSeq = calcularMedia(pontosPes, freqPes);
    double desvioPesSeq = calcularDesvioPadrao(pontosPes, freqPes, mediaPesSeq);
    double cvPesSeq = (desvioPesSeq / mediaPesSeq) * 100.0;
    auto fimSeq = chrono::high_resolution_clock::now();

    double tempoSeq = chrono::duration<double>(fimSeq - inicioSeq).count();

    // Calcular tempo paralelo (repetindo com paralelismo habilitado)
    auto inicioPar = chrono::high_resolution_clock::now();

    #pragma omp parallel
    {
        #pragma omp sections
        {
            #pragma omp section
            {
                gerarClasses(estaturas, 8.0, pontosEst, freqEst);
                double mediaEst = calcularMedia(pontosEst, freqEst);
                double desvioEst = calcularDesvioPadrao(pontosEst, freqEst, mediaEst);
                double cvEst = (desvioEst / mediaEst) * 100.0;
            }
            #pragma omp section
            {
                gerarClasses(pesos, 4.0, pontosPes, freqPes);
                double mediaPes = calcularMedia(pontosPes, freqPes);
                double desvioPes = calcularDesvioPadrao(pontosPes, freqPes, mediaPes);
                double cvPes = (desvioPes / mediaPes) * 100.0;
            }
        }
    }

    auto fimPar = chrono::high_resolution_clock::now();
    double tempoPar = chrono::duration<double>(fimPar - inicioPar).count();

    // Impressao organizada
    cout << fixed << setprecision(2);
    cout << "          RESULTADOS DO DESAFIO OPENMP       " << endl;
    cout << "---------------------------------------------" << endl;
    cout << "Total de pessoas: " << pessoas.size() << endl;
    cout << "---------------------------------------------" << endl;
    cout << "Estatura:" << endl;
    cout << "  Media: " << mediaEstSeq << " cm" << endl;
    cout << "  Desvio padrao: " << desvioEstSeq << " cm" << endl;
    cout << "  Coeficiente de variacao: " << cvEstSeq << " %" << endl;
    cout << "---------------------------------------------" << endl;
    cout << "Peso:" << endl;
    cout << "  Media: " << mediaPesSeq << " kg" << endl;
    cout << "  Desvio padrao: " << desvioPesSeq << " kg" << endl;
    cout << "  Coeficiente de variacao: " << cvPesSeq << " %" << endl;
    cout << "---------------------------------------------" << endl;
    cout << "Tempo sequencial: " << tempoSeq << " s" << endl;
    cout << "Tempo paralelo:   " << tempoPar << " s" << endl;
    cout << "Speedup:          " << tempoSeq / tempoPar << "x" << endl;
    cout << "---------------------------------------" << endl;
    return 0;
}
