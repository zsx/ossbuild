#include <QtGui/QApplication>
#include "qrenderer.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));

    QString videolcoation = QFileDialog::getOpenFileName(0, "Select a video file", 
        ".", "Format (*.avi *.mkv *.ogg *.asf *.mov)");

    if (videolcoation.isEmpty())
        return -1;

    QRenderer w(videolcoation);
    w.setWindowTitle("glimagesink implements the gstxoverlay interface");

    return a.exec();
}
