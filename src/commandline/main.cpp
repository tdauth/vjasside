#include <iostream>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QFile>

#include "../app/vjassscanner.h"
#include "../app/vjassparser.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("vjassparser");
    QCoreApplication::setApplicationVersion("1.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));

    // Process the actual command line arguments given by the user
    parser.process(app);

    const QStringList args = parser.positionalArguments();

    if (args.isEmpty()) {
        std::cout << "Missing input sources!" << std::endl;
    } else {
        VJassScanner vjassScanner;
        VJassParser vjassParser;

        for (const QString &file : args) {
            QFile f(file);

            if (f.open(QFile::ReadOnly | QFile::Text)) {
                QTextStream in(&f);
                QString input = in.readAll();

                QList<VJassToken> tokens = vjassScanner.scan(input, false);
                VJassAst *ast = vjassParser.parse(tokens);

                const QList<VJassParseError> errors = ast->getAllParseErrors();

                if (errors.isEmpty()) {
                    std::cout << "No syntax errors in file " << file.toStdString() << std::endl;
                } else {
                    std::cout << "Found " << errors.length() << " syntax errors in file " << file.toStdString() << ": " << std::endl;

                    for (const VJassParseError &error : errors) {
                        std::cout << "Syntax error at line " << (error.getLine() + 1) << " and column " << (error.getColumn() + 1) << ": " << error.getError().toStdString() << std::endl;
                    }
                }

                delete ast;
                ast = nullptr;
            }
        }
    }

    return app.exec();
}
