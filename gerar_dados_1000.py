import random

# contagens calculadas a partir da sua tabela
c1 = 200  # classe 150-154
c2 = 267  # classe 154-158
c3 = 533  # classe 158-162

lines = []

# opcao A: peso uniforme entre 45 e 100 kg
for _ in range(c1):
    est = round(random.uniform(150.0, 154.0), 1)
    peso = round(random.uniform(45.0, 100.0), 1)
    lines.append(f"{est} {peso}")

for _ in range(c2):
    est = round(random.uniform(154.0, 158.0), 1)
    peso = round(random.uniform(45.0, 100.0), 1)
    lines.append(f"{est} {peso}")

for _ in range(c3):
    est = round(random.uniform(158.0, 162.0), 1)
    peso = round(random.uniform(45.0, 100.0), 1)
    lines.append(f"{est} {peso}")

# embaralha as linhas para nao ficar agrupado
random.shuffle(lines)

with open("dados.txt", "w") as f:
    for l in lines:
        f.write(l + "\n")

print("Arquivo dados.txt gerado com 1000 linhas (opcao A: peso uniforme).")
