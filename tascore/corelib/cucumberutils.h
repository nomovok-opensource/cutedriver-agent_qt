#ifndef CUCUMBERUTILS_H
#define CUCUMBERUTILS_H

#include <QString>
#include <QHash>

class CucumberUtils
{
public:


    struct StepData {
        QString stepFileSpec;
        QString regExp;
        QString text;
        QHash<QString, QString> flags;

        StepData() {}
        StepData(const StepData &other) : stepFileSpec(other.stepFileSpec), regExp(other.regExp), text(other.text), flags(other.flags) {}
        void clear() { stepFileSpec.clear(); regExp.clear(); text.clear(); flags.clear(); }
        QString getPluginName() { return stepFileSpec.left(stepFileSpec.indexOf('.')); }

    };

    typedef QHash<QString, StepData> StepDataMap;

    CucumberUtils();

    static StepDataMap readSteps(const QString &pluginName);
    static StepDataMap readAllSteps();

private:
    static void addFileContents(StepDataMap &map, const QString &filePath);

};

#endif // CUCUMBERUTILS_H
