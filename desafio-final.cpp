#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <omp.h>
#include <iomanip>
#include <chrono>

using namespace std;

struct Pessoa {
    double estatura;
    double peso;
};

//  FUNÇÕES AUXILIARES 

// Média ponderada
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

// Desvio padrão populacional
double calcularDesvioPadrao(const vector<double>& pontos, const vector<int>& freq, double media) {
    double soma = 0.0;
    int total = 0;

    #pragma omp parallel for reduction(+:soma,total)
    for (int i = 0; i < (int)pontos.size(); i++) {
        soma += freq[i] * pow(pontos[i] - media, 2);
        total += freq[i];
    }

    return sqrt(soma / total);
}

// Função para gerar frequências conforme intervalos fixos
void gerarClassesFixas(const vector<double>& dados, const vector<pair<double, double>>& intervalos,
                       vector<double>& pontosMedios, vector<int>& freq) {
    int numClasses = intervalos.size();
    pontosMedios.resize(numClasses);
    freq.assign(numClasses, 0);

    // Calcula o ponto médio de cada classe
    for (int i = 0; i < numClasses; i++) {
        pontosMedios[i] = (intervalos[i].first + intervalos[i].second) / 2.0;
    }

    // Conta quantos dados estão em cada intervalo
    #pragma omp parallel for
    for (int i = 0; i < (int)dados.size(); i++) {
        double valor = dados[i];
        for (int j = 0; j < numClasses; j++) {
            if (valor >= intervalos[j].first && valor < intervalos[j].second) {
                #pragma omp atomic
                freq[j]++;
                break;
            }
        }
    }
}

int main() {
    vector<Pessoa> pessoas;
    ifstream arquivo("dados.txt");

    if (!arquivo.is_open()) {
        cout << "Erro ao abrir o arquivo 'dados.txt'!" << endl;
        return 1;
    }

    double est, pes;
    while (arquivo >> est >> pes) {
        pessoas.push_back({est, pes});
    }
    arquivo.close();

    if (pessoas.empty()) {
        cout << "Arquivo vazio ou formato incorreto!" << endl;
        return 1;
    }

    vector<double> estaturas, pesos;
    for (auto& p : pessoas) {
        estaturas.push_back(p.estatura);
        pesos.push_back(p.peso);
    }

    // === INTERVALOS FIXOS ===
    vector<pair<double, double>> intervalosEst = {
        {150.0, 154.0},
        {154.0, 158.0},
        {158.0, 162.0}
    };
    vector<pair<double, double>> intervalosPes = {
        {45.0, 49.0},
        {49.0, 53.0},
        {53.0, 57.0},
        {57.0, 61.0},
        {61.0, 65.0},
        {65.0, 69.0},
        {69.0, 73.0},
        {73.0, 77.0},
        {77.0, 81.0},
        {81.0, 85.0},
        {85.0, 89.0},
        {89.0, 93.0},
        {93.0, 97.0},
        {97.0, 101.0}
    };

    vector<double> pontosEst, pontosPes;
    vector<int> freqEst, freqPes;

    cout << fixed << setprecision(2);

    //SEQUENCIAL 
    auto inicioSeq = chrono::high_resolution_clock::now();

    gerarClassesFixas(estaturas, intervalosEst, pontosEst, freqEst);
    gerarClassesFixas(pesos, intervalosPes, pontosPes, freqPes);

    double mediaEstSeq = calcularMedia(pontosEst, freqEst);
    double desvioEstSeq = calcularDesvioPadrao(pontosEst, freqEst, mediaEstSeq);
    double cvEstSeq = (desvioEstSeq / mediaEstSeq) * 100.0;

    double mediaPesSeq = calcularMedia(pontosPes, freqPes);
    double desvioPesSeq = calcularDesvioPadrao(pontosPes, freqPes, mediaPesSeq);
    double cvPesSeq = (desvioPesSeq / mediaPesSeq) * 100.0;

    auto fimSeq = chrono::high_resolution_clock::now();
    double tempoSeq = chrono::duration<double>(fimSeq - inicioSeq).count();

    //PARALELO
    auto inicioPar = chrono::high_resolution_clock::now();

    double mediaEstPar, desvioEstPar, cvEstPar;
    double mediaPesPar, desvioPesPar, cvPesPar;

    #pragma omp parallel sections
    {
        #pragma omp section
        {
            gerarClassesFixas(estaturas, intervalosEst, pontosEst, freqEst);
            mediaEstPar = calcularMedia(pontosEst, freqEst);
            desvioEstPar = calcularDesvioPadrao(pontosEst, freqEst, mediaEstPar);
            cvEstPar = (desvioEstPar / mediaEstPar) * 100.0;
        }
        #pragma omp section
        {
            gerarClassesFixas(pesos, intervalosPes, pontosPes, freqPes);
            mediaPesPar = calcularMedia(pontosPes, freqPes);
            desvioPesPar = calcularDesvioPadrao(pontosPes, freqPes, mediaPesPar);
            cvPesPar = (desvioPesPar / mediaPesPar) * 100.0;
        }
    }

    auto fimPar = chrono::high_resolution_clock::now();
    double tempoPar = chrono::duration<double>(fimPar - inicioPar).count();

    //RESULTADOS 
    cout << "\n========== RESULTADOS DO DESAFIO OPENMP ==========" << endl;
    cout << "Total de pessoas: " << pessoas.size() << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "ESTATURA (intervalos fixos)" << endl;
    cout << "  Media: " << mediaEstSeq << " cm" << endl;
    cout << "  Desvio padrao: " << desvioEstSeq << " cm" << endl;
    cout << "  CV: " << cvEstSeq << " %" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "PESO (intervalos de 4 kg)" << endl;
    cout << "  Media: " << mediaPesSeq << " kg" << endl;
    cout << "  Desvio padrao: " << desvioPesSeq << " kg" << endl;
    cout << "  CV: " << cvPesSeq << " %" << endl;
    cout << "--------------------------------------------------" << endl;
    cout << "Tempo sequencial: " << tempoSeq << " s" << endl;
    cout << "Tempo paralelo:   " << tempoPar << " s" << endl;
    cout << "Speedup obtido:   " << tempoSeq / tempoPar << "x" << endl;
    cout << "==================================================" << endl;

    return 0;
}
