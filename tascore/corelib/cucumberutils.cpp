#include "cucumberutils.h"
#include <testabilitysettings.h>

#include <QDirIterator>
#include <QFile>
#include <QRegExp>

CucumberUtils::CucumberUtils()
{
}


CucumberUtils::StepDataMap CucumberUtils::readSteps(const QString &pluginName)
{
    StepDataMap ret;

    QString path = TestabilitySettings::getBasePath("cucumber");
    QDirIterator it(path, QDirIterator::Subdirectories);
    const QString prefix = pluginName + ".";
    static const QString suffix = ".steps";

    for (QString filePath = it.next()
         ; !filePath.isEmpty()
         ; filePath = it.next() ) {

        QString fileName = it.fileName();
        if (fileName.endsWith(suffix) && fileName.startsWith(prefix)  && it.fileInfo().isFile()) {
            addFileContents(ret, it.fileInfo().canonicalPath());
        }
    }

    return ret;
}


CucumberUtils::StepDataMap CucumberUtils::readAllSteps()
{
    StepDataMap ret;

    QString path = TestabilitySettings::getBasePath("cucumber");
    QDirIterator it(path, QDirIterator::Subdirectories);
    static const QString suffix = ".steps";

    for (QString filePath = it.next()
         ; !filePath.isEmpty()
         ; filePath = it.next() ) {

        if (it.fileName().endsWith(suffix) && it.fileInfo().isFile()) {
            addFileContents(ret, it.fileInfo().canonicalPath());
        }
    }

    return ret;
}


static void parseFlags(QHash<QString, QString> &flagMap, QStringList flagList)
{
    foreach(const QString &pair, flagList) {
        int pos = pair.indexOf("=");
        // don't care if flag name is actually empty, and with duplicate flags, overwrite earlier value with new
        if (pos == -1)
            flagMap.insert(pair, QString()); // flag without value
        else
            flagMap.insert(pair.left(pos), pair.mid(pos+1)); // flag with value
    }
}


static void insertStepData(CucumberUtils::StepDataMap &map, CucumberUtils::StepData &data)
{
    if (!data.regExp.isEmpty()) {
        map.insert(data.regExp, data);
    }
    // else ignore incomplete data
}


void CucumberUtils::addFileContents(CucumberUtils::StepDataMap &map, const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QFile::ReadOnly)) {

        const QString fileSpecBase = filePath + ":";

        bool lastEmpty = true;
        QString lastLine;
        StepData data;
        int lineNum = 0;

        while (!file.atEnd()) {
            QString line = file.readLine();
            ++lineNum;
            QString trimmedLine = line.trimmed();

            // skip empty lines
            if (trimmedLine.isEmpty()) {
                lastEmpty = true;
                continue;
            }

            // check for new step
            // format of STEP line: STEP [flag[=value]]...
            if (lastEmpty && trimmedLine.startsWith("STEP")) {
                static const QRegExp whiteSpaceEx("\\s+");
                QStringList splitLine = trimmedLine.split(whiteSpaceEx, QString::SkipEmptyParts);

                if (!splitLine.isEmpty() && splitLine.takeFirst() == "STEP") {

                    // start of next step, store current step data and prepare for new step
                    insertStepData(map, data);
                    data.clear();

                    // get step flags
                    parseFlags(data.flags, splitLine);
                    data.stepFileSpec = fileSpecBase + QString::number(lineNum);

                    lastEmpty = false;
                    continue;
                }
            }

            lastEmpty = false;

            // first non-empty line of step is the regExp, rest are step text
            if (data.regExp.isEmpty()) {
                data.regExp = trimmedLine;
            }
            else {
                data.text.append(lastLine);
            }

        }

        // add last item
        insertStepData(map, data);

        file.close();
    }
}
