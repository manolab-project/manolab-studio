#include "CourseWindow.h"
#include "imgui.h"
#include "JsonReader.h"
#include "Util.h"
#include "HttpProtocol.h"
#include "TcpClient.h"
#include "HttpClient.h"
#include "Log.h"

static const char gDefaultAddress[] = "mdbn.site";
static const char gDefaulPort[] = "80";
static const char gDefaultPath[] = "/docs/mdbn/gestion/envoi_course.php";

CourseWindow::CourseWindow()
{
    memcpy(mBufAddress, gDefaultAddress, sizeof(gDefaultAddress));
    memcpy(mPath, gDefaultPath, sizeof(gDefaultPath));
    memcpy(mPort, gDefaulPort, sizeof(gDefaulPort));
}

bool CourseWindow::GetCourse(const std::string &host, const std::string &path, uint16_t port)
{
    bool success = false;
    HttpClient client;
    std::string response;
    if (client.Get(host, path, port, response))
    {
        std::cout << response << std::endl;
        JsonReader reader;
        JsonValue json;
        if (reader.ParseString(json, response))
        {
            if (json.IsArray())
            {
                mTable.clear();
                mCategories.clear();
                for (const auto &e : json.GetArray())
                {
                    Entry entry;

                    entry.dossard = Util::FromString<uint64_t>(e.FindValue("dossard").GetString());
                    entry.dbId = Util::FromString<uint64_t>(e.FindValue("id").GetString());
                    entry.category = e.FindValue("F5").GetString();
                    entry.lastname = e.FindValue("F6").GetString();
                    entry.firstname = e.FindValue("F7").GetString();
                    entry.club = e.FindValue("F8").GetString();
                    mTable[entry.dossard] = entry;

                    mCategories.insert(entry.category);
                }

                success = true;
            }
            else
            {
                TLogError("[HTTP] JSON format: not an array!");
            }
        }
        else
        {
            TLogError("[HTTP] Parse JSON reply error");
        }
    }
    else
    {
        TLogError("[HTTP] Receive timeout !!");
    }
    return success;
}

void CourseWindow::Draw(const char *title, bool *p_open, IProcessEngine &engine)
{
    ImGui::Begin(title, p_open);

    /* ======================  Réception de la course depuis le Cloud ====================== */
    ImGui::PushItemWidth(200);
    ImGui::InputText("Adresse du serveur",  mBufAddress, sizeof(mBufAddress));
    ImGui::SameLine();
    ImGui::InputText("Chemin",  mPath, sizeof(mPath));

    ImGui::PopItemWidth();
    ImGui::PushItemWidth(100);
    ImGui::InputText("Port",  mPort, sizeof(mPort), ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button( "Récupérer", ImVec2(100, 40)))
    {
        uint16_t port = Util::FromString<uint16_t>(mPort);
        if (GetCourse(mBufAddress, mPath, port))
        {
            std::vector<Value> line;
            for (auto & c : mCategories)
            {
                line.push_back(c);
            }
            engine.SetTableEntry("categories", 0, line);

            uint32_t index = 0;
            for (const auto & e : mTable)
            {
                line.clear();

                line.push_back(Value(e.second.dossard));
                line.push_back(Value(e.second.category));
                engine.SetTableEntry("dossards", index, line);
                index++;
            }
        }
    }

    ImGui::Text("Participants : %d, Catégories : %d", static_cast<int>(mTable.size()), static_cast<int>(mCategories.size()));

    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | 
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | 
                ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;

    if (ImGui::BeginTable("table1", 5, tableFlags))
    {
        ImGui::TableSetupColumn("Id", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Dossard", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Catégorie", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Nom", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Prénom", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();

        for (const auto & e : mTable)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", std::to_string(e.second.dbId).c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%s", std::to_string(e.second.dossard).c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%s", e.second.category.c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%s", e.second.lastname.c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text("%s", e.second.firstname.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
