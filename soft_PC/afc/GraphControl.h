#pragma once
#include <windows.h>
#include <vector>
#include <algorithm>

/*
оновлення даних для графіка:
std::vector<GraphControl::PointFA> data;
data.push_back({100.0, 0.5});
data.push_back({200.0, 0.8});
GraphControl::SetData(hGraph, data);
*/
//-----------------------
extern HWND hStatus;
//----------------------

class GraphControl
{
public:
	/// --- Simple struct to hold point data (frequency and amplitude) ---
    struct PointFA
    {
        double x;
        double y;
    };
	// --- Static method to register the window class for the graph control ---
    static void Register(HINSTANCE hInst)
    {
        WNDCLASS wc = {};
        wc.lpfnWndProc = WndProc;
        wc.hInstance = hInst;
        wc.lpszClassName = L"GraphControl";
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        RegisterClass(&wc);
    }
	// --- Static method to update graph data from outside the control ---
    static void SetData(HWND hWnd, const std::vector<PointFA>& newData)
    {
        auto* graph = (GraphControl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!graph) return;
        graph->data = newData;
        InvalidateRect(hWnd, NULL, TRUE);
    }
	//гетер і сетер для bsnapToPoints
    static void SetSnapToPoints(HWND hWnd, bool value)
    {
        auto* graph = (GraphControl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!graph) return;
        graph->bsnapToPoints = value;
        InvalidateRect(hWnd, NULL, TRUE);
    }
    static bool GetSnapToPoints(HWND hWnd)
    {
        auto* graph = (GraphControl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!graph) return false;
        return graph->bsnapToPoints;
	}
    //гетер і сетер для bOnlyVout
    static void SetOnlyVout(HWND hWnd, bool value)
    {
        auto* graph = (GraphControl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!graph) return;
        graph->bOnlyVout = value;
        InvalidateRect(hWnd, NULL, TRUE);
    }
    static bool GetOnlyVout(HWND hWnd)
    {
        auto* graph = (GraphControl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
        if (!graph) return false;
        return graph->bOnlyVout;
    }

private:
    std::vector<PointFA> data;
    // --- Store graph width for mouse interaction calculations ---
    int graphWidth;
	int graphHeight;
    // --- For snapToPoints mode: index of the currently selected point ---
    int selectedIndex = -1;
    //курсор ходить по точках графіку або довільно (false)
    bool bsnapToPoints = true;
    //показувати тільки вихідне 0....1
    bool bOnlyVout = true;


	// --- For mouse interaction (optional) ---
    bool mouseInside = false;
    POINT mousePos = {};
    double minX = 0, maxX = 1;
    double minY = 0, maxY = 1;
	
    // --- Graph margins ---
    const int margin = 40;
	// --- Helper to calculate "nice" step size for axis ticks ---
    double NiceStep(double range, int targetTicks = 8)
    {
        double roughStep = range / targetTicks;
        double mag = pow(10.0, floor(log10(roughStep)));
        double norm = roughStep / mag;

        double step;
        if (norm < 1.5) step = 1.0;
        else if (norm < 3.0) step = 2.0;
        else if (norm < 7.0) step = 5.0;
        else step = 10.0;

        return step * mag;
    }
	// --- Find nearest data point index to a given X value (for mouse interaction) ---
    int FindNearestPoint(double mouseXValue)
    {
        if (data.empty()) return -1;

        int left = 0;
        int right = (int)data.size() - 1;

        // Бінарний пошук
        while (left <= right)
        {
            int mid = (left + right) / 2;

            if (data[mid].x < mouseXValue)
                left = mid + 1;
            else
                right = mid - 1;
        }

        // left — перша більша
        int idx1 = max(0, min(left, (int)data.size() - 1));
        int idx2 = max(0, idx1 - 1);

        if (fabs(data[idx1].x - mouseXValue) <
            fabs(data[idx2].x - mouseXValue))
            return idx1;
        else
            return idx2;
    }
    //----------------------------------------------------------------------------------
	// --- Main window procedure for the graph control ---
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        GraphControl* graph = (GraphControl*)GetWindowLongPtr(hWnd, GWLP_USERDATA);

        switch (msg)
        {
		/// --- Create and destroy graph control instance ---
        case WM_CREATE:
        {
            graph = new GraphControl();
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)graph);
            //get graph client area height and width
            RECT rcClient;
            GetClientRect(hWnd, &rcClient);
            graph->graphHeight = rcClient.bottom - rcClient.top;
            graph->graphWidth = rcClient.right - rcClient.left;
            return 0;
        }
		// --- Clean up on destroy ---
        case WM_DESTROY:
        {
            delete graph;
            return 0;
        }
		/// --- Optional mouse handling for interactivity ---
        case WM_MOUSEMOVE:
        {
			//курсор ходить по точках графіку, а не довільно
            if (GetSnapToPoints(hWnd)) {
                if (!graph || graph->data.empty()) break;

                graph->mouseInside = true;

                int mx = GET_X_LPARAM(lParam);

                // Перерахунок пікселів у X-значення
                double rangeX = graph->maxX - graph->minX;
                double scaleX = (graph->graphWidth - 2.0 * graph->margin) / rangeX;

                double mouseXValue =
                    graph->minX +
                    (mx - graph->margin) / scaleX;

                graph->selectedIndex =
                    graph->FindNearestPoint(mouseXValue);

                TRACKMOUSEEVENT tme = {};
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);

                InvalidateRect(hWnd, NULL, FALSE);
                return 0;
            }
			//курсор ходить довільно, а не по точках графіку
            else {
                if (!graph) break;
                graph->mouseInside = true;
                graph->mousePos.x = GET_X_LPARAM(lParam);
                graph->mousePos.y = GET_Y_LPARAM(lParam);
                TRACKMOUSEEVENT tme = {};
                tme.cbSize = sizeof(TRACKMOUSEEVENT);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hWnd;
                TrackMouseEvent(&tme);
                InvalidateRect(hWnd, NULL, FALSE);
                return 0;
            }
        }
        // Handle mouse leave to reset state
        case WM_MOUSELEAVE:
        {
            if (!graph) break;
            graph->mouseInside = false;
            InvalidateRect(hWnd, NULL, FALSE);
            return 0;
        }
		// --- Main painting logic ---
        case WM_PAINT:
        {
            if (!graph) break;

            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);

            RECT rc;
            GetClientRect(hWnd, &rc);

            int width = rc.right - rc.left;
            int height = rc.bottom - rc.top;

            // --- Double Buffer ---
            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBmp = CreateCompatibleBitmap(hdc, width, height);
            SelectObject(memDC, memBmp);

            HBRUSH bg = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &rc, bg);
            DeleteObject(bg);

            // --- Axes ---
            HPEN axisPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(memDC, axisPen);

            // X axis
            MoveToEx(memDC, graph->margin, height - graph->margin, NULL);
            LineTo(memDC, width - graph->margin, height - graph->margin);

            // ---- Y Axis scale ----
            double rangeY = graph->maxY - graph->minY;
            double stepY = graph->NiceStep(rangeY);
			// --- Draw Y axis ticks and labels ---
            HPEN tickPenY = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(memDC, tickPenY);
			// --- Set transparent background for text ---
            SetBkMode(memDC, TRANSPARENT);
			// --- Loop through "nice" tick positions on Y axis ---
            for (double yVal =
                ceil(graph->minY / stepY) * stepY;
                yVal <= graph->maxY;
                yVal += stepY)
            {
                int y = height - graph->margin -
                    (int)((yVal - graph->minY) *
                        (height - 2.0 * graph->margin) / rangeY);

                // tick
                MoveToEx(memDC, graph->margin - 5, y, NULL);
                LineTo(memDC, graph->margin, y);

                // текст
                wchar_t buffer[64];
                swprintf_s(buffer, L"%.2f", yVal);

                SIZE sz;
                GetTextExtentPoint32(memDC, buffer, lstrlen(buffer), &sz);

                TextOut(memDC,
                    graph->margin - 8 - sz.cx,
                    y - sz.cy / 2,
                    buffer,
                    lstrlen(buffer));
            }
            DeleteObject(tickPenY);


            // Y axis -----------------------------------------
            MoveToEx(memDC, graph->margin, graph->margin, NULL);
            LineTo(memDC, graph->margin, height - graph->margin);

            DeleteObject(axisPen);

			// --- Draw ticks and labels ----------------------------
            // ---- X Axis scale ----
            double rangeX = graph->maxX - graph->minX;
            double stepX = graph->NiceStep(rangeX);
			// --- Draw X axis ticks and labels ---
            HPEN tickPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
            SelectObject(memDC, tickPen);
			// --- Set transparent background for text ---
            SetBkMode(memDC, TRANSPARENT);
			// --- Loop through "nice" tick positions on X axis ---
            for (double xVal =
                ceil(graph->minX / stepX) * stepX;
                xVal <= graph->maxX;
                xVal += stepX)
            {
                int x = graph->margin +
                    (int)((xVal - graph->minX) *
                        (width - 2.0 * graph->margin) / rangeX);

                // tick
                MoveToEx(memDC, x, height - graph->margin, NULL);
                LineTo(memDC, x, height - graph->margin + 5);

                // текст
                wchar_t buffer[64];

                if (graph->maxX > 1000.0)
                    swprintf_s(buffer, L"%.1f kHz", xVal / 1000.0);
                else
                    swprintf_s(buffer, L"%.0f Hz", xVal);
				// --- Center text horizontally ---
                SIZE sz;
                GetTextExtentPoint32(memDC, buffer, lstrlen(buffer), &sz);
				// --- Draw label below the tick ---
                TextOut(memDC, x - sz.cx / 2, height - graph->margin + 8, buffer,
                    lstrlen(buffer));
            }

            DeleteObject(tickPen);


			if (!graph->data.empty())
            {
                double minX = graph->data[0].x;
                double maxX = graph->data[0].x;
                double minY = graph->data[0].y;
                double maxY = graph->data[0].y;

                for (auto& p : graph->data)
                {
                    minX = min(minX, p.x);
                    maxX = max(maxX, p.x);
                    minY = min(minY, p.y);
                    maxY = max(maxY, p.y);
                }

                if (maxX == minX) maxX += 1.0;
                if (maxY == minY) maxY += 1.0;
				
                // Store for potential future use (e.g., mouse interaction)
                graph->minX = minX;
                graph->maxX = maxX;
                graph->minY = minY;
                graph->maxY = maxY;
                //тільки вихідне від 0 до 1
                if (graph->bOnlyVout) {
                    graph->minY = 0;
                    graph->maxY = 1;
                }

				// --- Scale factors ---
                double scaleX = (width - 2.0 * graph->margin) / (maxX - minX);
                double scaleY = (height - 2.0 * graph->margin) / (maxY - minY);
				// --- Draw graph line ---
                HPEN graphPen = CreatePen(PS_SOLID, 2, RGB(0, 100, 255));
                SelectObject(memDC, graphPen);
				// Start at the first point
                int x0 = graph->margin + (int)((graph->data[0].x - minX) * scaleX);
                int y0 = height - graph->margin -
                    (int)((graph->data[0].y - minY) * scaleY);

                MoveToEx(memDC, x0, y0, NULL);

                for (size_t i = 1; i < graph->data.size(); i++)
                {
                    int x = graph->margin +
                        (int)((graph->data[i].x - minX) * scaleX);

                    int y = height - graph->margin -
                        (int)((graph->data[i].y - minY) * scaleY);

                    LineTo(memDC, x, y);
                }

                DeleteObject(graphPen);
            }
			// --- If snapToPoints is enabled, show crosshair and coordinates for the nearest data point ---
            if (GetSnapToPoints(hWnd)) {
                if (graph->mouseInside && graph->selectedIndex >= 0)
                {
                    auto& p = graph->data[graph->selectedIndex];

                    double rangeX = graph->maxX - graph->minX;
                    double rangeY = graph->maxY - graph->minY;

                    int x = graph->margin +
                        (int)((p.x - graph->minX) *
                            (width - 2.0 * graph->margin) / rangeX);

                    int y = height - graph->margin -
                        (int)((p.y - graph->minY) *
                            (height - 2.0 * graph->margin) / rangeY);

                    HPEN crossPen = CreatePen(PS_SOLID, 1, RGB(200, 0, 0));
                    SelectObject(memDC, crossPen);

                    // Вертикальна лінія
                    MoveToEx(memDC, x, graph->margin, NULL);
                    LineTo(memDC, x, height - graph->margin);

                    // Горизонтальна
                    MoveToEx(memDC, graph->margin, y, NULL);
                    LineTo(memDC, width - graph->margin, y);

                    // Точка
                    Ellipse(memDC, x - 4, y - 4, x + 4, y + 4);

                    DeleteObject(crossPen);

                    wchar_t buffer[128];
                    swprintf_s(buffer, L"F=%.1f Hz   A=%.3f", p.x, p.y);
                    SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(1, 0), (LPARAM)buffer);

                    //SetBkMode(memDC, TRANSPARENT);
                    //TextOut(memDC, x + 10, y - 20, buffer, lstrlen(buffer));
                }
            }
			//курсор ходить довільно, а не по точках графіку
            else {
                if (graph->mouseInside)
                {
                    int mx = graph->mousePos.x;
                    int my = graph->mousePos.y;

                    // Малюємо лінії
                    HPEN crossPen = CreatePen(PS_DOT, 1, RGB(200, 0, 0));
                    SelectObject(memDC, crossPen);

                    MoveToEx(memDC, mx, graph->margin, NULL);
                    LineTo(memDC, mx, height - graph->margin);

                    MoveToEx(memDC, graph->margin, my, NULL);
                    LineTo(memDC, width - graph->margin, my);

                    DeleteObject(crossPen);

                    // Перерахунок координат
                    double scaleX = (width - 2.0 * graph->margin) / (graph->maxX - graph->minX);
                    double scaleY = (height - 2.0 * graph->margin) / (graph->maxY - graph->minY);

                    double valueX = graph->minX + (mx - graph->margin) / scaleX;
                    double valueY = graph->minY +
                        (height - graph->margin - my) / scaleY;

                    wchar_t buffer[128];
                    swprintf_s(buffer, L"X=%.2f  Y=%.2f", valueX, valueY);
                    SendMessageW(hStatus, SB_SETTEXTW, MAKEWPARAM(1, 0), (LPARAM)buffer);

                    //SetBkMode(memDC, TRANSPARENT);
                    //TextOut(memDC, mx + 10, my - 20, buffer, lstrlen(buffer));
                }
            }




			// --- Blit to screen ---
            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
			// --- Cleanup ---
            DeleteObject(memBmp);
            DeleteDC(memDC);

            EndPaint(hWnd, &ps);
            return 0;
        }
        }

        return DefWindowProc(hWnd, msg, wParam, lParam);
    }
};
