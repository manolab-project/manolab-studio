#include "CourseWindow.h"
#include "imgui.h"
#include "JsonReader.h"
#include "Util.h"
#include "HttpProtocol.h"
#include "TcpClient.h"
#include "Log.h"

static const char gDefaultAddress[] = "127.0.0.1";
static const char gDefaulPort[] = "8000";
static const char gDefaultPath[] = "/envoi_course.php";

CourseWindow::CourseWindow()
{
    memcpy(mBufAddress, gDefaultAddress, sizeof(gDefaultAddress));
    memcpy(mPath, gDefaultPath, sizeof(gDefaultPath));
    memcpy(mPort, gDefaulPort, sizeof(gDefaulPort));
}

void CourseWindow::GetCourse(const std::string &host, const std::string &path, uint16_t port)
{
    HttpRequest request;
    HttpProtocol proto;
    tcp::TcpClient client;

    request.method = "GET";
    request.protocol = "HTTP/1.1";
    request.query = path;
    request.headers["Host"] = host;
    request.headers["Content-type"] = "application/json";
    // request.headers["Content-length"] = std::to_string(body.size());

    client.Initialize();
    if (client.Connect(host, port))
    {
        if (client.Send(proto.GenerateRequest(request)))
        {
            TLogInfo("[HTTP] Send request success!");
            
            std::string output;
            if (client.RecvWithTimeout(output, 100*1024, 5000))
            {
                // std::cout << output << "\r\n-----------------\r\n" << std::endl;
              //  std::cout << "Size :" << output.size() << "\r\n" << std::endl;
                HttpReply reply;
                proto.ParseReplyHeader(output, reply);
                JsonReader reader;
                JsonValue json;
                if (reader.ParseString(json, reply.body))
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
        }
        else
        {
            TLogError("[HTTP] Send request failed");
        }
    }
    else
    {
        TLogError("[HTTP] Connect to server failed");
    }
}

void CourseWindow::Draw(const char *title, bool *p_open)
{
    ImGui::Begin(title, p_open);

    /* ======================  Réception de la course depuis le Cloud ====================== */
    ImGui::PushItemWidth(200);
    ImGui::InputText("Adresse du serveur : ",  mBufAddress, sizeof(mBufAddress));
    ImGui::SameLine();
    ImGui::InputText("Adresse du serveur : ",  mPath, sizeof(mPath));
        
    ImGui::PopItemWidth();
    ImGui::PushItemWidth(100);
    ImGui::InputText("Port",  mPort, sizeof(mPort), ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button( "Récupérer", ImVec2(100, 40)))
    {
        uint16_t port = Util::FromString<uint16_t>(mPort);
        GetCourse(mBufAddress, mPath, port);

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
