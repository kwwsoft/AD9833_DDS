#include "framework.h"
#include <commctrl.h>
#include <windowsx.h>
#include <string>
#include <codecvt>
#include <algorithm>
#include "IniStoreSetting.h"
#include "EditFreqSubClass.h"

//-----------------------------------------------------------------------------
#define MAX_DIGITS 8   // формат: 2 + 3 + 3 = 8 цифр
//-----------------------------------------------------------------------------
extern HWND hWndMain;                       // main window handle
extern HWND hEditFreqFrom;                     // edit частота з
extern HWND hEditFreqTo;                       // edit частота до
extern HWND hComboStepCount;                   // combo кількість кроків
extern HWND hstaticStepCount;                  // static кількість кроків
extern HWND hButtonStart;                      // button старт
extern HWND hButtonStop;                       // button стоп
extern UINT32 userSettings[3]; // 0 - frequency from, 1 - frequency to, 2 - step
//-----------------------------------------------------------------------------
// дістаємо лише цифри з текста
std::string ExtractDigits(const std::string& text)
{
    std::string digits;
    for (char c : text)
        if (isdigit((unsigned char)c))
            digits += c;

    if (digits.length() > MAX_DIGITS)
        digits.resize(MAX_DIGITS);

    return digits;
}
//-----------------------------------------------------------------------------
// форматуємо стрічку у вигляді 
std::string FormatDigits(const std::string& digits)
{
    std::string result, result1;
	result1 = std::string(digits.rbegin(), digits.rend()); // перевертаємо строку для зручності форматування
	// додаємо пробіли після цифри з кінця
    for (size_t i = result1.length(); i > 0; --i)
    {
        result += result1[i-1];

        if (i == 7 || i == 4)
        {
            result += ' ';
        }
    }
    return result;
}
//-----------------------------------------------------------------------------
// заповнення користувацького масиву в одній функції для зручності
void UpdateUserSettingsFromEdit(HWND hwnd, int index, std::string& digits) {
    char buffer[64];
	// Отримуємо текст з поля редагування
    GetWindowTextA(hwnd, buffer, sizeof(buffer));
	// Дістаємо лише цифри з тексту
    std::string oldText = buffer;
	// Оновлюємо userSettings відповідно до індексу (0 для "From", 1 для "To")
    digits = ExtractDigits(oldText);
    if (digits.empty()) {
        userSettings[index] = 0; // Сбрасываем значение в userSettings при фокусе на пустом поле
        SetWindowTextW(hstaticStepCount, L"Step count: N/A"); // Сбрасываем текст количества шагов при фокусе на пустом поле
    }
	//якщо поле не пусте, зберігаємо числове значення в userSettings і оновлюємо кількість кроків, якщо обидві частоти встановлені
    else
    {
		//зберігаємо числове значення в userSettings
        userSettings[index] = std::stoul(digits); 
		//перевіряємо логічну послідовність "From" і "To"
        if (userSettings[1] != 0 && userSettings[0] > userSettings[1]) {
            // Если значение "From" больше "To", сбрасываем "To" и количество шагов
            userSettings[1] = 0;
            SetWindowTextA(hEditFreqTo, ""); // Очищаем поле "To"
			//зкидаємо кількість кроків і текст кількості кроків, оскільки логічна послідовність порушена
            SetWindowTextW(hstaticStepCount, L"Step count: N/A");
			EnableWindow(hButtonStart, FALSE); // Деактивируем кнопку "Start" при нарушении логической последовательности
			EnableWindow(hButtonStop, FALSE); // Деактивируем кнопку "Stop" при нарушении логической последовательности
        }
        else {
			// якщо обидві частоти встановлені і логічна послідовність не порушена, оновлюємо кількість кроків
            if (userSettings[1] != 0) {
				// Обчислюємо кількість кроків і оновлюємо текст
                unsigned long stepCount = (userSettings[1] - userSettings[0]) / (userSettings[2] != 0 ? userSettings[2] : 1);
                std::wstring stepCountText = L"Step count: " + std::to_wstring(stepCount);
				// Оновлюємо текст кількості кроків
                SetWindowTextW(hstaticStepCount, std::wstring(stepCountText.begin(), stepCountText.end()).c_str());
				EnableWindow(hButtonStart, TRUE); // Активируем кнопку "Start" при правильной логической последовательности
            }
        }
    }
}
// 
//-----------------------------------------------------------------------------
// Подкласс для редактирования частоты "From"
LRESULT CALLBACK EditFreqFromSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_SETFOCUS:
    {
		std::string digits;
        digits.resize(MAX_DIGITS);
		UpdateUserSettingsFromEdit(hwnd, 0, digits);
        SetWindowTextA(hwnd, digits.c_str());
        break;
    }
    case WM_KILLFOCUS:
    {
        std::string digits;
        digits.resize(MAX_DIGITS);
        UpdateUserSettingsFromEdit(hwnd, 0, digits);
        IniStoreSetting::SaveFrequencyFrom(digits);
		std::string formatted = FormatDigits(digits);
		SetWindowTextA(hwnd, formatted.c_str());
        break;
    }
    case WM_CHAR:
    {
        if (wParam == VK_ESCAPE)
        {
            // Сбрасываем текст к пустой строке при нажатии Escape
            SetWindowTextA(hwnd, "");
			EnableWindow(hButtonStart, FALSE); // Деактивируем кнопку "Start" при сбросе текста
			EnableWindow(hButtonStop, FALSE); // Деактивируем кнопку "Stop" при сбросе текста
            return 0;
		}
		//TAB дозволяє переходити між полями, тому не блокуємо його
        if (wParam == VK_TAB || wParam == VK_RETURN)
        {
			SetFocus(hEditFreqTo); // переміщаємо фокус на наступне поле
			return 0;
        }
        if (!isdigit((unsigned char)wParam) && wParam != VK_BACK && wParam != VK_DELETE)
        {
            return 0; // блокуємо все крім цифр
        }
        break;
    }
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}
//-----------------------------------------------------------------------------
// Подкласс для редактирования частоты "To"
LRESULT CALLBACK EditFreqToSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_SETFOCUS:
    {
        std::string digits;
        digits.resize(MAX_DIGITS);
        UpdateUserSettingsFromEdit(hwnd, 1, digits);
        SetWindowTextA(hwnd, digits.c_str());
        break;
    }
    case WM_KILLFOCUS:
    {
        std::string digits;
        digits.resize(MAX_DIGITS);
        UpdateUserSettingsFromEdit(hwnd, 1, digits);
        IniStoreSetting::SaveFrequencyTo(digits);
        std::string formatted = FormatDigits(digits);
        SetWindowTextA(hwnd, formatted.c_str());
        break;
    }
    case WM_CHAR:
    {
        if (wParam == VK_ESCAPE)
        {
            // Сбрасываем текст к пустой строке при нажатии Escape
            SetWindowTextA(hwnd, "");
            EnableWindow(hButtonStart, FALSE); // Деактивируем кнопку "Start" при сбросе текста
            EnableWindow(hButtonStop, FALSE); // Деактивируем кнопку "Stop" при сбросе текста
            return 0;
        }
        //TAB дозволяє переходити між полями, тому не блокуємо його
        if (wParam == VK_TAB || wParam == VK_RETURN)
        {
            SetFocus(hComboStepCount); // переміщаємо фокус на наступне поле
            return 0;
        }
        if (!isdigit((unsigned char)wParam) && wParam != VK_BACK && wParam != VK_DELETE)
        {
            return 0; // блокуємо все крім цифр
        }
        break;
    }
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}
//-----------------------------------------------------------------------------
// Подклас для вибору кроку по частоті
LRESULT CALLBACK ComboStepCountSubClass(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
    switch (uMsg)
    {
    case WM_COMMAND:
    {
        if (HIWORD(wParam) == CBN_SELCHANGE)
        {
            //позиція вибратого тексту кроку в боксі
            LRESULT sel = SendMessage(hwnd, CB_GETCURSEL, 0, 0);
            if (sel != CB_ERR)
            {
                wchar_t buffer[64];
				buffer[63] = 0;
                //забираємо в буфер числове значення чатоти
                SendMessage(hwnd, CB_GETLBTEXT, sel, (LPARAM)buffer);
                std::wstring selectedText(buffer);
                //зберегти в налаштування
                IniStoreSetting::SaveFrequencyStep(selectedText);
				//зберігаємо числове значення частоти в userSettings[2] і оновлюємо кількість кроків, якщо обидві частоти встановлені
                userSettings[2] = std::stoul(std::string(selectedText.begin(), selectedText.end())); 
                if (userSettings[0] != 0 && userSettings[1] != 0) {
                    uint16_t stepCount = (userSettings[1] - userSettings[0]) / userSettings[2];
                    std::string stepCountText = "Step count: " + std::to_string(stepCount);
                    SetWindowTextW(hstaticStepCount, std::wstring(stepCountText.begin(), stepCountText.end()).c_str());
                }
            }
        }
        break;
	}

    case WM_CHAR:
    {
        if (wParam == VK_ESCAPE)
        {
            // Сбрасываем выбор к первому элементу при нажатии Escape
            SendMessage(hwnd, CB_SETCURSEL, 0, 0);
            return 0;
        }
        if (wParam == VK_TAB || wParam == VK_RETURN)
        {
            SetFocus(hButtonStart); // переміщаємо фокус на наступне поле
            return 0;
		}
        break;
    }
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}
//-----------------------------------------------------------------------------






