# TOMWare
TOMWare (Transparency and Overhead Measurement for Malware)

A ferramenta TOMWare comtém contramedidas para mitigar algumas técnicas de anti-instrumentação baseadas em overhead e transparência, empregadas por esses malwares para detectar a presença de frameworks de instrumentação binária dinâmica (DBI). 

---

## Estrutura do repositório

```text
TOMWare/                              ← Diretório principal do repositório.
├── Resultados                        ← Resultados dos experimentos do artigo
|   ├── Apps-Teste/                   ← Contém os executáveis das aplicações de teste
|   ├── Apps-Teste-src/               ← Contém o código fonte das aplicações de teste
|   ├── Capturas-Tela/                ← Capturas de tela dos experimentos
|   └── Resultados/                   ← Artwuivos dos resultados dos experimentos do artigo
├── TOMWare/                          ← código-fonte (.cpp/.h)
└── pin/                              ← Intel Pin 3.28 descompactado
```

---

## Dependências para compilação (Windows)

| Ferramenta | Versão mínima | Observações |
|------------|---------------|-------------|
| **Visual Studio 2019** (ou 2022) | Community/Pro | Baixe em <https://visualstudio.microsoft.com/pt-br/vs/older-downloads/>. Instale o *Desktop C++ Workload*. No VS 2022 marque o **toolset v142** para manter compatibilidade com o Pin. |
| **Windows 10 SDK** | 10.0.19041 | Vem com o instalador do VS. |
| **Intel Pin** | 3.28 (x64, MSVC) | Baixe em <https://software.intel.com/sites/landingpage/pintool/downloads/pin-3.28-98749-g6643ecee5-msvc-windows.zip> e extraia em `pin\` (mova **somente** o conteúdo evitando manter a subpasta gerada pela extração). |

---

## Compilação

1. Abra **`TOMWare.sln`** no Visual Studio.
   *Quando o VS 2022 perguntar para “atualizar o toolset”, escolha **Não**. Mantenha **Visual Studio 2019 (v142)** para garantir compatibilidade com o Pin*
2. Selecione **Configuration → Debug** e **Platform → x64**.
3. Compile (**Ctrl + Shift + B**).

O arquivo resultante será gerado em:

```
x64\Debug\TOMWare.dll
```

---

## Execução

### Parâmetros comuns

| Parâmetro | Descrição                                                                                         |
| --------- | ------------------------------------------------------------------------------------------------- |
|   `-da`   | Ativa todas as contramedidas disponíveis.                                                         |
|   `-de`   | Ativa apenas a contramedida SanitizePinEnvVars.                                                   |
|   `-dm`   | Ativa apenas a contramedida InstMemcmpMask.                                                       |
|   `-do`   | Ativa apenas a contramedida SkewMask; requer o simulador de overhead para avaliação.              |
|   `-go`   | Liga o gerador de overhead incorporado, útil para demonstrar a eficácia da contramedida SkewMask. |

### Sintaxe básica

> ⚠️ **Atenção:** Substitua os placeholders (PATH_PIN_x64, PATH_TOMWARE) e o caminho do programa alvo pelos caminhos reais dos artefatos.

```powershell
<PATH_PIN_x64>\pin.exe -t <PATH_TOMWARE>\TOMWare.dll -da -de -dm -do -go -- C:\Samples\alvo.exe
```
---