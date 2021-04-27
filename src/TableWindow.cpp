#include "TableWindow.h"
#include "imgui.h"
#include "JsonReader.h"
#include "Util.h"
#include "HttpProtocol.h"
#include "TcpClient.h"
#include "Log.h"

static const char *gDefaultResultServer = "http://";

TableWindow::TableWindow()
{
    RefreshWindowParameter();

    memcpy(mBufAddress, gDefaultResultServer, sizeof(gDefaultResultServer));
}

void TableWindow::RefreshWindowParameter()
{
    std::string tempWindow = std::to_string(mWindow/1000);
    sprintf(buf2, "%.10s", tempWindow.c_str());
}

void TableWindow::SendToServer(const std::string &body, const std::string &host)
{
    HttpRequest request;

    request.method = "POST";
    request.protocol = "HTTP/1.1";
    request.query = "/reception_resultats.php";
    request.body = body;
    request.headers["Host"] = "www." + host;
    request.headers["Content-type"] = "application/csv";
    request.headers["Content-length"] = std::to_string(body.size());

    tcp::TcpClient client;
    HttpProtocol http;

    client.Initialize();
    if (client.Connect(host, 8123))
    {
        if (client.Send(http.GenerateRequest(request)))
        {
            TLogInfo("[HTTP] Send request success!");
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

std::string TableWindow::ToCSV(const std::map<int64_t, Entry> &table, int64_t startTime)
{
    std::string csv;

    csv = "Tag, Tours, Temps\r\n";

    for (const auto &t : table)
    {
        std::stringstream ss;

        ss << t.first << ", "
           << t.second.laps.size() << ", "
           << t.second.ToString(startTime)
           << std::endl;

        csv += ss.str();
    }
    return csv;
}


void TableWindow::Draw(const char *title, bool *p_open)
{
    // Local copy to shorter mutex locking
    mMutex.lock();
    std::map<int64_t, Entry> table = mTable;
    int64_t startTime = mStartTime;
    mMutex.unlock();

    ImGui::Begin(title, p_open);

    /* ======================  Export CSV ====================== */
    ImGui::Text("Tableau des passages");
    ImGui::SameLine();
    if (ImGui::Button( "Export CSV", ImVec2(100, 40)))
    {
        Util::StringToFile("export.csv", ToCSV(table, startTime), false);
    }

    /* ======================  Envoi dans le Cloud ====================== */
    ImGui::PushItemWidth(200);
    ImGui::InputText("Adresse d'envoi des résultats",  mBufAddress, sizeof(mBufAddress));
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button( "Envoyer", ImVec2(100, 40)))
    {
        SendToServer(ToCSV(table, startTime), "127.0.0.1");
    }

    ImGui::Text("Tags : %d", static_cast<int>(table.size()));

    /* ======================  PARAMETRAGE ====================== */
    ImGui::PushItemWidth(200);
    ImGui::InputText("Fenêtre de blocage (en secondes)",  buf2, sizeof(buf2), ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button( "Appliquer", ImVec2(100, 40)))
    {
       std::scoped_lock<std::mutex> lock(mMutex);
       mWindow = Util::FromString<int64_t>(buf2) * 1000; // en millisecondes
    }


    ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | 
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | 
                ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti;

    if (ImGui::BeginTable("table1", 4, tableFlags))
    {
        ImGui::TableSetupColumn("Participant", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Numéro", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Tours", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Temps", ImGuiTableColumnFlags_WidthStretch);

        ImGui::TableHeadersRow();

        for (const auto & e : table)
        {
             ImGui::TableNextRow();

             ImGui::TableSetColumnIndex(1);
             ImGui::Text("%s", std::to_string(e.second.tag).c_str());

             ImGui::TableSetColumnIndex(2);
             ImGui::Text("%s", std::to_string(e.second.laps.size()).c_str());

             ImGui::TableSetColumnIndex(3);

             ImGui::Text("%s", e.second.ToString(startTime).c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void TableWindow::ParseAction(const std::vector<Value> &args)
{
    JsonReader reader;
    JsonValue json;

    if (args.size() > 0)
    {
        if (reader.ParseString(json, args[0].GetString()))
        {
            Entry e;
            e.tag = json.FindValue("tag").GetInteger64();
            int64_t time = json.FindValue("time").GetInteger64();

            mMutex.lock();

            if (mTable.size() == 0)
            {
                mStartTime = time;
            }

            if (mTable.count(e.tag) > 0)
            {
                std::vector<int64_t> &l = mTable[e.tag].laps;
                if (l.size() > 0)
                {
                    int64_t diff = time - l[l.size() - 1];

                    if (diff > mWindow)
                    {
                        mTable[e.tag].laps.push_back(time);
                    }
                }

            }
            else
            {
                e.laps.push_back(time);
                mTable[e.tag] = e;
            }

            mMutex.unlock();
        }
    }
}
