// pie menu test
// v0.00

#include <imgui.h>
#include <imgui_internal.h>

// Return >= 0 on mouse release
// Optional int* p_selected display and update a currently selected item
int PiePopupSelectMenu(const ImVec2& center, const char* popup_id, const char** items, int items_count, int* p_selected)
{
    int ret = -1;
    
    // FIXME: Missing a call to query if Popup is open so we can move the PushStyleColor inside the BeginPopupBlock (e.g. IsPopupOpen() in imgui.cpp)
    // FIXME: Our PathFill function only handle convex polygons, so we can't have items spanning an arc too large else inner concave edge artifact is too visible, hence the ImMax(7,items_count)
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));
    if (ImGui::BeginPopup(popup_id))
    {
        const ImVec2 drag_delta = ImVec2(ImGui::GetIO().MousePos.x - center.x, ImGui::GetIO().MousePos.y - center.y);
        const float drag_dist2 = drag_delta.x*drag_delta.x + drag_delta.y*drag_delta.y;

        const ImGuiStyle& style = ImGui::GetStyle();
        const float RADIUS_MIN = 30.0f;
        const float RADIUS_MAX = 120.0f;
        const float RADIUS_INTERACT_MIN = 20.0f;
        const int ITEMS_MIN = 6;

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        //ImGuiWindow* window = ImGui::GetCurrentWindow();
        draw_list->PushClipRectFullScreen();
        draw_list->PathArcTo(center, (RADIUS_MIN + RADIUS_MAX)*0.5f, 0.0f, IM_PI*2.0f*0.99f, 32);   // FIXME: 0.99f look like full arc with closed thick stroke has a bug now
        draw_list->PathStroke(ImColor(0,0,0), true, RADIUS_MAX - RADIUS_MIN);

        const float item_arc_span = 2*IM_PI / ImMax(ITEMS_MIN, items_count);
        float drag_angle = atan2f(drag_delta.y, drag_delta.x);
        if (drag_angle < -0.5f*item_arc_span)
            drag_angle += 2.0f*IM_PI;
        //ImGui::Text("%f", drag_angle);    // [Debug]
        
        int item_hovered = -1;
        for (int item_n = 0; item_n < items_count; item_n++)
        {
            const char* item_label = items[item_n];
            const float item_ang_min = item_arc_span * (item_n+0.02f) - item_arc_span*0.5f; // FIXME: Could calculate padding angle based on how many pixels they'll take
            const float item_ang_max = item_arc_span * (item_n+0.98f) - item_arc_span*0.5f;

            bool hovered = false;
            if (drag_dist2 >= RADIUS_INTERACT_MIN*RADIUS_INTERACT_MIN)
            {
                if (drag_angle >= item_ang_min && drag_angle < item_ang_max)
                    hovered = true;
            }
            bool selected = p_selected && (*p_selected == item_n);

            int arc_segments = (int)(32 * item_arc_span / (2*IM_PI)) + 1;
            draw_list->PathArcTo(center, RADIUS_MAX - style.ItemInnerSpacing.x, item_ang_min, item_ang_max, arc_segments);
            draw_list->PathArcTo(center, RADIUS_MIN + style.ItemInnerSpacing.x, item_ang_max, item_ang_min, arc_segments);
            //draw_list->PathFill(window->Color(hovered ? ImGuiCol_HeaderHovered : ImGuiCol_FrameBg));
            draw_list->PathFill(hovered ? ImColor(100,100,150) : selected ? ImColor(120,120,140) : ImColor(70,70,70));
 
            ImVec2 text_size = ImGui::GetWindowFont()->CalcTextSizeA(ImGui::GetWindowFontSize(), FLT_MAX, 0.0f, item_label);
            ImVec2 text_pos = ImVec2(
                center.x + cosf((item_ang_min + item_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.x * 0.5f,
                center.y + sinf((item_ang_min + item_ang_max) * 0.5f) * (RADIUS_MIN + RADIUS_MAX) * 0.5f - text_size.y * 0.5f);
            draw_list->AddText(text_pos, ImColor(255,255,255), item_label);

            if (hovered)
                item_hovered = item_n;
        }
        draw_list->PopClipRect();

        if (ImGui::IsMouseReleased(0))
        {
            ImGui::CloseCurrentPopup();
            ret = item_hovered;
            if (p_selected)
                *p_selected = item_hovered;
        }
        ImGui::EndPopup();
    }
    ImGui::PopStyleColor(2);
    return ret;
}

// Test code
void func()
{
  static const char* test_data = "Menu";
  const char* items[] = { "Orange", "Blue", "Purple", "Gray", "Yellow", "Las Vegas" };
  int items_count = sizeof(items)/sizeof(*items);

  static int selected = -1;

  ImGui::Button(selected >= 0 ? items[selected] : "Menu", ImVec2(50,50));
  if (ImGui::IsItemActive())          // Don't wait for button release to activate the pie menu
      ImGui::OpenPopup("##piepopup");
  
  ImVec2 pie_menu_center = ImGui::GetIO().MouseClickedPos[0];
  int n = PiePopupSelectMenu(pie_menu_center, "##piepopup", items, items_count, &selected);
  if (n >= 0)
      printf("returned %d\n", n);
}
