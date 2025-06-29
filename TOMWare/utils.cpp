// utils.cpp
#include "utils.h"


bool IsPointer(ADDRINT value) {
    // Verifica se o valor � um ponteiro acess�vel
    return PIN_CheckReadAccess(reinterpret_cast<void*>(value));
}

bool IsStringPointer(UINT64 addr) {
    if (!IsPointer(addr)) {
        return false; // Ponteiro inv�lido
    }

    char* str = reinterpret_cast<char*>(addr);
    for (int i = 0; i < 256; ++i) { // Limite para evitar leitura fora do contexto
        if (!PIN_CheckReadAccess(&str[i])) {
            return false; // Parte da string n�o � acess�vel
        }
        if (str[i] == '\0') {
            return true; // Encontrou o final da string
        }
        if (str[i] < 0x20 || str[i] > 0x7E) {
            return false; // N�o � um caractere imprim�vel
        }
    }
    return false; // N�o encontrou um final de string v�lido
}


bool IsValidWideString(const std::wstring& str) {
    // Verifique primeiro o tamanho m�nimo
    if (str.size() < 3) {
        return false;
    }

    for (wchar_t ch : str) {
        // Permitir apenas caracteres ASCII alfanum�ricos e os permitidos extras
        if (!((ch >= L'0' && ch <= L'9') || // N�meros
            (ch >= L'A' && ch <= L'Z') || // Letras mai�sculas
            (ch >= L'a' && ch <= L'z') || // Letras min�sculas
            ch == L'.' || ch == L'-' || ch == L'_' ||
            ch == L':' || ch == L'\\' || ch == L'/')) {
            return false; // Caractere inv�lido encontrado
        }
    }

    return true; // Todos os caracteres s�o v�lidos
}


bool IsValidString(const std::string& str) {
    // Verifique primeiro o tamanho m�nimo
    if (str.size() < 3) {
        return false;
    }

    for (char ch : str) {
        // Permitir apenas caracteres ASCII alfanum�ricos e os extras permitidos
        if (!((ch >= '0' && ch <= '9') || // N�meros
            (ch >= 'A' && ch <= 'Z') || // Letras mai�sculas
            (ch >= 'a' && ch <= 'z') || // Letras min�sculas
            ch == '.' || ch == '-' || ch == '_' ||
            ch == ':' || ch == '\\' || ch == '/')) {
            return false; // Caractere inv�lido encontrado
        }
    }

    return true; // Todos os caracteres s�o v�lidos
}

std::wstring CopyLPCWSTR(ADDRINT addr) {
    const size_t MAX_STRING_LENGTH = 800; // Defina o tamanho m�ximo da string
    wchar_t buffer[MAX_STRING_LENGTH];    // Buffer para armazenar a string copiada
    memset(buffer, 0, sizeof(buffer));    // Inicializa o buffer com zeros

    size_t copied = 0;                    // Contador de caracteres copiados

    for (size_t i = 0; i < MAX_STRING_LENGTH - 1; i++) {
        // Use PIN_SafeCopy para copiar um �nico caractere wide por vez (2 bytes em UTF-16/UCS-2)
        size_t result = PIN_SafeCopy(&buffer[i], reinterpret_cast<const void*>(addr + i * sizeof(wchar_t)), sizeof(wchar_t));

        // Se a c�pia falhar ou encontrar o terminador nulo, finalize
        if (result != sizeof(wchar_t) || buffer[i] == L'\0') {
            break;
        }

        copied++;
    }

    // Verifica se o tamanho m�ximo foi excedido
    if (copied == MAX_STRING_LENGTH - 1) {
        //std::wcerr << L"Warning: Wide string copy reached the maximum limit." << std::endl;
    }

    return std::wstring(buffer); // Retorna a string wide copiada como um std::wstring
}

std::string CopyLPCSTR(ADDRINT addr) {
    const size_t MAX_STRING_LENGTH = 800; // Defina o tamanho m�ximo da string
    char buffer[MAX_STRING_LENGTH];       // Buffer para armazenar a string copiada
    memset(buffer, 0, sizeof(buffer));    // Inicializa o buffer com zeros

    size_t copied = 0;                    // Contador de bytes copiados

    for (size_t i = 0; i < MAX_STRING_LENGTH - 1; i++) {
        // Use PIN_SafeCopy para copiar um �nico caractere por vez
        size_t result = PIN_SafeCopy(&buffer[i], reinterpret_cast<const void*>(addr + i), 1);

        // Se a c�pia falhar ou encontrar o terminador nulo, finalize
        if (result == 0 || buffer[i] == '\0') {
            break;
        }

        copied++;
    }

    // Verifica se o tamanho m�ximo foi excedido
    if (copied == MAX_STRING_LENGTH - 1) {
        //std::cerr << "Warning: String copy reached the maximum limit." << std::endl;
    }

    return std::string(buffer); // Retorna a string copiada como um std::string
}

std::string GetStringValueFromRegister(UINT64 value, UINT32 size) {
    std::ostringstream result;

    // Caso de um �nico caractere (1 byte)
    if (size == 1) {
        char c = static_cast<char>(value & 0xFF); // Apenas o byte menos significativo
        if (c >= 0x20 && c <= 0x7E) { // Verifica se � imprim�vel
            result << "'" << c << "'";
        }
        else {
            result << "";
        }
        return result.str();
    }

    // Caso de sequ�ncia de caracteres (at� 8 bytes)
    const char* str = reinterpret_cast<const char*>(&value);
    bool isString = true;
    for (UINT32 i = 0; i < size && i < 8; ++i) { // Limite do registrador � 8 bytes
        if (str[i] == '\0') { // Termina��o de string
            break;
        }
        if (str[i] < 0x20 || str[i] > 0x7E) { // N�o imprim�vel
            isString = false;
            break;
        }
    }

    if (isString) {
        result << "\"" << std::string(str, size).c_str() << "\""; // Converte para string
    }
    else {
        result << "";
    }

    return result.str();
}

size_t GetSafeWStringLength(ADDRINT addr, size_t maxLen) {
    size_t length = 0;
    wchar_t ch = 0;

    for (; length < maxLen; ++length) {
        // Copia apenas 1 wchar_t por vez de forma segura
        if (PIN_SafeCopy(&ch, reinterpret_cast<void*>(addr + length * sizeof(wchar_t)), sizeof(wchar_t)) != sizeof(wchar_t)) {
            break; // Falha ao acessar a mem�ria -> parar
        }

        if (ch == L'\0') {
            break; // Fim da string
        }
    }

    return length;
}

std::wstring ConvertAddrToWideStringSafe(ADDRINT addr, size_t maxLen) {
    std::wstring result;

    size_t length = GetSafeWStringLength(addr, maxLen);
    if (length == 0) return result;

    // Buffer tempor�rio com terminador adicional
    std::vector<wchar_t> buffer(length + 1, L'\0');

    // Use &buffer[0] no lugar de buffer.data() � compat�vel com C++11
    size_t copied = PIN_SafeCopy(&buffer[0], reinterpret_cast<void*>(addr), (length + 1) * sizeof(wchar_t));
    if (copied >= length * sizeof(wchar_t)) {
        result = std::wstring(&buffer[0]);
    }

    return result;
}


std::wstring ConvertAddrToWideString(ADDRINT addr) {
    if (addr != 0) {
        wchar_t* wideCharStr = reinterpret_cast<wchar_t*>(addr);
        return std::wstring(wideCharStr);
    }
    else {
        return std::wstring();  // Retorna uma string vazia se o endere�o for nulo
    }
}

std::string ConvertAddrToAnsiString(ADDRINT addr) {
    if (addr != 0) {
        const char* charStr = reinterpret_cast<const char*>(addr);
        return std::string(charStr);
    }
    else {
        return std::string();  // Retorna uma string vazia se o endere�o for nulo
    }
}

std::string WStringToString(const std::wstring& wstr)
{
    if (wstr.empty())
    {
        return std::string();
    }

    // Obt�m o tamanho necess�rio do buffer para a string convertida (em UTF-8)
    int sizeNeeded = WindowsAPI::WideCharToMultiByte(
        CP_UTF8,          // C�digo de p�gina para UTF-8
        0,                // Nenhum flag extra
        wstr.data(),      // Ponteiro para a string wide char
        static_cast<int>(wstr.size()), // Comprimento da string wide char
        nullptr,          // Sem buffer de sa�da por enquanto (queremos saber o tamanho necess�rio)
        0,
        nullptr,          // Sem uso de caractere de substitui��o
        nullptr           // Sem retorno de caractere substituto usado
    );

    if (sizeNeeded <= 0)
    {
        return std::string();
    }

    // Cria um buffer de tamanho adequado para a string resultante
    std::string result(sizeNeeded, 0);

    // Converte de wide char para string em UTF-8
    WindowsAPI::WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr.data(),
        static_cast<int>(wstr.size()),
        &result[0],
        sizeNeeded,
        nullptr,
        nullptr
    );

    return result;
}

void LogWString(const std::wstring& wstr) {
    std::string str = WStringToString(wstr);
    LOG("Converted String: %s\n", str.c_str());
}

std::string ExtractModuleName(const std::string& fullPath) {
    size_t lastSlashPos = fullPath.find_last_of("\\/");
    if (lastSlashPos != std::string::npos) {
        return fullPath.substr(lastSlashPos + 1);
    }
    else {
        return fullPath; // Retorna o fullPath original se n�o houver separador de diret�rio
    }
}

std::string toUpperCase(const std::string& str) {
    std::string upperCaseStr = str; // Cria uma c�pia da string original
    for (char& c : upperCaseStr) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
    return upperCaseStr;
}

VOID PrintTrace(std::deque<std::string>& threadTraces) {
    std::cout << "Routine Trace:" << std::endl;
    for (const auto& name : threadTraces) {
        std::cout << "  " << name << std::endl;
    }
}


void GetParentProcessName() {
    using namespace WindowsAPI;
    DWORD currentProcessId = GetCurrentProcessId();
    DWORD parentProcessId = 0;

    // Tirar um snapshot de todos os processos
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Erro ao criar snapshot.\n";
        return;
    }

    PROCESSENTRY32 pe32 = { sizeof(PROCESSENTRY32) };

    if (Process32First(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == currentProcessId) {
                parentProcessId = pe32.th32ParentProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }

    CloseHandle(hSnapshot);

    if (parentProcessId != 0) {
        // Tirar outro snapshot para procurar o processo pai
        hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot == INVALID_HANDLE_VALUE) {
            std::cerr << "Erro ao criar snapshot.\n";
            return;
        }

        if (Process32First(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == parentProcessId) {
                    std::wcout << L"Nome do processo pai: " << pe32.szExeFile << L"\n";
                    break;
                }
            } while (Process32Next(hSnapshot, &pe32));
        }

        CloseHandle(hSnapshot);
    }
    else {
        std::cerr << "Processo pai n�o encontrado.\n";
    }
}

std::string getFileName(const std::string& filePath) {
    // Encontra a �ltima ocorr�ncia de '\' ou '/'
    size_t pos = filePath.find_last_of("\\/");

    if (pos != std::string::npos) {
        return filePath.substr(pos + 1);
    }

    return filePath;
}