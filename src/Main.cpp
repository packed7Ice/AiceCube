#include "MainComponent.h"
#include <juce_gui_extra/juce_gui_extra.h> // DocumentWindow など

class AiceCubeApplication : public juce::JUCEApplication {
public:
  const juce::String getApplicationName() override { return "AiceCube"; }
  const juce::String getApplicationVersion() override { return "0.1.0"; }
  bool moreThanOneInstanceAllowed() override { return true; }

  void initialise(const juce::String &) override {
    mainWindow.reset(new MainWindow(getApplicationName()));
  }

  void shutdown() override { mainWindow = nullptr; }

  void systemRequestedQuit() override { quit(); }

  void anotherInstanceStarted(const juce::String &) override {}

  class MainWindow : public juce::DocumentWindow {
  public:
    MainWindow(const juce::String &name)
        : DocumentWindow(name, juce::Colours::darkgrey,
                         DocumentWindow::allButtons) {
      setUsingNativeTitleBar(true);
      setContentOwned(new MainComponent(), true);

      centreWithSize(getWidth(), getHeight());
      setResizable(true, true);
      setVisible(true);
    }

    void closeButtonPressed() override {
      juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
  };

private:
  std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(AiceCubeApplication)
