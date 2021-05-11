#ifndef TABLE_WINDOW_H
#define TABLE_WINDOW_H

#include "Value.h"
#include "IProcessEngine.h"
#include <vector>
#include <map>
#include <mutex>

class TableWindow
{
public:
    TableWindow();
    void Draw(const char *title, bool *p_open, IProcessEngine& engine);

    void ParseAction(const std::vector<Value> &args);
private:
    struct Entry {
        int64_t tag;
        std::vector<int64_t> laps;

        std::string ToString(int64_t start) const
        {
            std::string times;
            for (const auto & l : laps)
            {
                double diff = static_cast<double>(l - start);
                diff /= 1000;
                times += std::to_string(diff) + ";  ";
            }
            return times;
        }
    };

    std::map<int64_t, Entry> mTable;
    std::mutex mMutex;
    int64_t mWindow = 5000;
    int64_t mStartTime = 0;
    char mBufAddress[200];
    char buf2[10];
    char mPath[200];
    char mPort[10];

    std::vector<Value> mCatLabels;
    // clé: nom catégorie
    // valeur: si autorisée ou non
    std::map<std::string, bool> mCategories;

    // clé: numéro de dossard
    // Valeur: catégorie
    std::map<uint32_t, std::string> mDossards;

    void RefreshWindowParameter();
    void SendToServer(const std::string &body, const std::string &host, const std::string &path, uint16_t port);
    std::string ToJson(const std::map<int64_t, Entry> &table, int64_t startTime);
};

#endif // TABLEWINDOW_H
