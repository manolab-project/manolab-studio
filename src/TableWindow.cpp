#include "TableWindow.h"
#include "imgui.h"
#include "JsonReader.h"
#include "Util.h"
#include "HttpProtocol.h"
#include "TcpClient.h"
#include "Log.h"

#include <random>

static const char gDefaultAddress[] = "mdbn.site";
static const char gDefaulPort[] = "80";
static const char gDefaultPath[] = "/docs/mdbn/gestion/reception_resultats.php";


TableWindow::TableWindow()
{
    memcpy(mBufAddress, gDefaultAddress, sizeof(gDefaultAddress));
    memcpy(mPath, gDefaultPath, sizeof(gDefaultPath));
    memcpy(mPort, gDefaulPort, sizeof(gDefaulPort));


    RefreshWindowParameter();
/*
    // FAKE DATA FOR TESTS
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis(1, 5000);

    // 10 coureurs
    for (int i = 0; i < 10; i++)
    {
        Entry e;
        e.tag = i + 1;
        for (int tours = 0; tours < 4; tours++)
        {
            uint64_t time = tours * 120000 + dis(gen);
            e.laps.push_back(time);
        }
        mTable[e.tag] = e;
    }
    */

}

void TableWindow::RefreshWindowParameter()
{
    std::string tempWindow = std::to_string(mWindow/1000);
    sprintf(buf2, "%.10s", tempWindow.c_str());
}

void TableWindow::SendToServer(const std::string &body, const std::string &host, const std::string &path, uint16_t port)
{
    HttpRequest request;

    request.method = "POST";
    request.protocol = "HTTP/1.1";
    request.query = path;
    request.body = body;
    request.headers["Host"] = "www." + host;
    request.headers["Content-type"] = "application/json";
    request.headers["Content-length"] = std::to_string(body.size());

    tcp::TcpClient client;
    HttpProtocol http;

    client.Initialize();
    if (client.Connect(host, port))
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

std::string TableWindow::ToJson(const std::map<int64_t, Entry> &table, int64_t startTime)
{
    JsonArray arr;

    for (const auto &t : table)
    {
        JsonObject obj;

        obj.AddValue("id", t.first);
        obj.AddValue("tours", static_cast<uint32_t>(t.second.laps.size()));
        obj.AddValue("temps", t.second.ToString(startTime));
        arr.AddValue(obj);
    }
    return arr.ToString();
}


void TableWindow::Draw(const char *title, bool *p_open, IProcessEngine &engine)
{
    // Local copy to shorter mutex locking
    mMutex.lock();
    std::map<int64_t, Entry> table = mTable;
    int64_t startTime = mStartTime;
    mMutex.unlock();

    ImGui::Begin(title, nullptr);

    /* ======================  Export CSV ====================== */
    ImGui::Text("Tableau des passages");
    ImGui::SameLine();
    if (ImGui::Button( "Export JSON", ImVec2(100, 40)))
    {
        Util::StringToFile("export.json", ToJson(table, startTime), false);
    }

    /* ======================  Envoi dans le Cloud ====================== */
    ImGui::PushItemWidth(200);
    ImGui::InputText("Adresse du serveur",  mBufAddress, sizeof(mBufAddress));
    ImGui::SameLine();
    ImGui::InputText("Chemin",  mPath, sizeof(mPath));

    ImGui::PopItemWidth();
    ImGui::PushItemWidth(100);
    ImGui::InputText("Port",  mPort, sizeof(mPort), ImGuiInputTextFlags_CharsDecimal);
    ImGui::PopItemWidth();
    ImGui::SameLine();
    if (ImGui::Button( "Envoyer", ImVec2(100, 40)))
    {
        uint16_t port = Util::FromString<uint16_t>(mPort);
        SendToServer(ToJson(table, startTime), mBufAddress, mPath, port);
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

    /* ======================  CATEGORIES ====================== */
    uint32_t nbLines = engine.GetTableSize("categories");
    if (nbLines == 1)
    {
        if (engine.GetTableEntry("categories", 0, mCatLabels))
        {
            // Les catégories ont changé
            if (mCatLabels.size() != mCategories.size())
            {
                mCategories.clear();
                for (const auto & c : mCatLabels)
                {
                    mCategories[c.GetString()] = false;
                }

                // On récupère tous les dossards et la catégorie associée
                nbLines = engine.GetTableSize("dossards");

                for (uint32_t i = 0; i < nbLines; i++)
                {
                    std::vector<Value> dossard;
                    engine.GetTableEntry("dossards", i, dossard);
                    if (dossard.size() == 2)
                    {
                        mDossards[dossard[0].GetInteger()] = dossard[1].GetString();
                    }
                }

            }
        }

        uint32_t index = 0;
        for (auto & c : mCategories)
        {
            ImGui::Checkbox(c.first.c_str(), &c.second);
            index++;
            if (index < mCategories.size())
            {
                ImGui::SameLine();
            }
        }
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

            // on récupère la catégorie de ce dossard (==tag)
            if (mDossards.count(e.tag) > 0)
            {
                std::string category = mDossards[e.tag];
                if (mCategories.count(category) > 0)
                {
                    if (mCategories[category])
                    {
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
                    else
                    {
                        TLogError("Catégorie " + category + " interdite");
                    }
                }
                else
                {
                    TLogError("Catégorie inconnue: " + category);
                }
            }
            else
            {
                TLogError("Dossard inconnu: " + std::to_string(e.tag));
            }
        }
    }
}
