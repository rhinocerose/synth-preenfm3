/*
 * Copyright 2021 Xavier Hosxe
 *
 * Author: Xavier Hosxe (xavier . hosxe (at) gmail . com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "FMDisplayMidiController.h"
#include "MidiControllerState.h"
#include "MidiControllerFile.h"
#include "TftDisplay.h"


#define X_OFFSET 16

FMDisplayMidiController::FMDisplayMidiController() {
    menuPressed_ = false;
    displayMode_ = DISPLAY_MIDI_CONTROLLER_MAIN;
    editLetterPosition_ = 0;
    midiChannel_ = 0;
    pageNumber_ = 0;
}

void FMDisplayMidiController::init(TftDisplay *tft, MidiControllerState *midiControllerState, MidiControllerFile *midiControllerFile) {
    tft_ = tft;
    midiControllerState_ = midiControllerState;
    midiControllerFile_ = midiControllerFile;

    midiControllerFile_->loadConfig(midiControllerState_);
}


void FMDisplayMidiController::refreshAllScreenByStep() {

    if (likely(refreshStatus_ != 21)) {
        if (unlikely(displayMode_ == DISPLAY_MIDI_CONTROLLER_EDIT_ENCODER)) {
            refreshAllScreenByStepEditEncoder();
            return;
        } else if (unlikely(displayMode_ == DISPLAY_MIDI_CONTROLLER_EDIT_BUTTON)) {
            refreshAllScreenByStepEditButton();
            return;
        } else if (unlikely(displayMode_ == DISPLAY_MIDI_CONTROLLER_SAVE)) {
            refreshAllScreenByStepSave();
            return;
        }
    }

    switch (refreshStatus_) {
        case 21: {
            tft_->pauseRefresh();
            tft_->clear();
            tft_->fillArea(0, 0, 240, 21, COLOR_DARK_BLUE);
            tft_->setCursorInPixel(8, 2);
            tft_->print("Midi Controller PFM3", COLOR_YELLOW, COLOR_DARK_BLUE);
            break;
        }
        case 20: {
            tft_->fillArea(0, 22, 240, 298, COLOR_BLACK);

            int y = 3 * TFT_BIG_CHAR_HEIGHT - 10;
            tft_->fillArea(0, y, 240, 126, COLOR_DARK_BLUE);
            tft_->fillArea(1, y + 1, 238, 124, COLOR_BLACK);

            y = 10 * TFT_BIG_CHAR_HEIGHT - 10;
            tft_->fillArea(0, y, 240, 126, COLOR_DARK_BLUE);
            tft_->fillArea(1, y + 1, 238, 124, COLOR_BLACK);

            displayMidiChannel();
            break;
        }
        case 19:
        case 18:
        case 17:
        case 16:
        case 15:
        case 14: {
            int encoderNumber = 19 - refreshStatus_;
            MidiEncoder* encoder = midiControllerState_->getEncoder(pageNumber_, encoderNumber);
            uint8_t x = (encoderNumber > 2 ? (encoderNumber - 3) * 7 : encoderNumber * 7) * TFT_BIG_CHAR_WIDTH + X_OFFSET;
            uint16_t y = (encoderNumber > 2 ? 6 : 3) * TFT_BIG_CHAR_HEIGHT;

            tft_->setCharColor(COLOR_WHITE);
            tft_->setCursorInPixel(x, y);
            tft_->print(encoder->name);

            tft_->setCharColor(COLOR_LIGHT_GRAY);
            tft_->setCursorInPixel(x + 2, y + TFT_BIG_CHAR_HEIGHT - 1);
            tft_->printSmallChars("cc");
            tft_->printSmallChar(encoder->controller);
            if (encoder->midiChannel != 16) {
                tft_->setCharColor(COLOR_ORANGE);
                tft_->printSmallChars(" ch");
                tft_->printSmallChar(encoder->midiChannel + 1);
            }

            displayEncoderValue(encoderNumber);
            break;
        }
        case 13:
        case 12:
        case 11:
        case 10:
        case 9:
        case 8: {
            int buttonNumber = 13 - refreshStatus_;
            MidiButton* button = midiControllerState_->getButton(pageNumber_, buttonNumber);

            uint8_t x = (buttonNumber > 2 ? (buttonNumber - 3) * 7 : buttonNumber * 7) * TFT_BIG_CHAR_WIDTH + X_OFFSET;
            uint16_t y = (buttonNumber > 2 ? 13 : 10) * TFT_BIG_CHAR_HEIGHT;

            tft_->setCharColor(COLOR_WHITE);
            tft_->setCursorInPixel(x, y);
            tft_->print(button->name);

            tft_->setCharColor(COLOR_LIGHT_GRAY);
            tft_->setCursorInPixel(x + 2, y + TFT_BIG_CHAR_HEIGHT - 1);
            tft_->printSmallChars("cc");
            tft_->printSmallChar(button->controller);
            if (button->midiChannel != 16) {
                tft_->setCharColor(COLOR_ORANGE);
                tft_->printSmallChars(" ch");
                tft_->printSmallChar(button->midiChannel + 1);
            }
            displayButtonValue(buttonNumber);
            break;
        }

        case 7:
            refreshStatus_ = 0;
            break;
    }

    if (refreshStatus_ > 0) {
        refreshStatus_--;
    }

    if (refreshStatus_ == 0) {
        tft_->restartRefreshTft();
    }
}


void FMDisplayMidiController::refreshAllScreenByStepEditEncoder() {
    switch (refreshStatus_) {
        case 20:
            tft_->fillArea(0, 22, 240, 298, COLOR_BLACK);
            tft_->fillArea(0, 50, 240, 250, COLOR_RED);
            tft_->fillArea(2, 72, 236, 216, COLOR_BLACK);
            break;
        case 19:
            tft_->setCharBackgroundColor(COLOR_RED);
            tft_->setCharColor(COLOR_BLACK);
            tft_->setCursorInPixel(54, 52);
            tft_->print("Edit Encoder");
            tft_->setCharBackgroundColor(COLOR_BLACK);
            break;
        case 18:
            tft_->setCursor(1,6);
            tft_->setCharColor(COLOR_DARK_GRAY);
            tft_->printSmallChars("a,d");
            tft_->setCursor(4, 6);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("Name    : ");
            displayEncoderParam(editControl_, ENCODER_PARAM_NAME);
            break;
        case 17:
            tft_->setCursor(1, 8);
            tft_->setCharColor(COLOR_DARK_GRAY);
            tft_->printSmallChar('b');
            tft_->setCursor(4, 8);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("Midi ch : ");
            displayEncoderParam(editControl_, ENCODER_PARAM_MIDI_CHANNEL);
            break;
        case 16:
            tft_->setCursor(1, 9);
            tft_->setCharColor(COLOR_DARK_GRAY);
            tft_->printSmallChar('e');

            tft_->setCursor(4, 9);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("CC      : ");
            displayEncoderParam(editControl_, ENCODER_PARAM_CONTROLLER);
            break;
        case 15:
            tft_->setCursor(1, 11);
            tft_->setCharColor(COLOR_DARK_GRAY);
            tft_->printSmallChar('c');

            tft_->setCursor(4, 11);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("Min     : ");
            displayEncoderParam(editControl_, ENCODER_PARAM_MIN);
            break;
        case 14:
            tft_->setCursor(1, 12);
            tft_->setCharColor(COLOR_DARK_GRAY);
            tft_->printSmallChar('f');

            tft_->setCursor(4, 12);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("Max     : ");
            displayEncoderParam(editControl_, ENCODER_PARAM_MAX);
            break;
        case 13:
            refreshStatus_ = 0;
            break;
    }


    if (refreshStatus_ > 0) {
        refreshStatus_--;
    }

    if (refreshStatus_ == 0) {
        tft_->restartRefreshTft();
    }
}



void FMDisplayMidiController::refreshAllScreenByStepEditButton() {
    switch (refreshStatus_) {
        case 20:
            tft_->fillArea(0, 22, 240, 298, COLOR_BLACK);
            tft_->fillArea(0, 50, 240, 250, COLOR_ORANGE);
            tft_->fillArea(2, 72, 236, 216, COLOR_BLACK);
        break;
        case 19:
            tft_->setCharBackgroundColor(COLOR_ORANGE);
            tft_->setCharColor(COLOR_BLACK);
            tft_->setCursorInPixel(59, 52);
            tft_->print("Edit Button");
            tft_->setCharBackgroundColor(COLOR_BLACK);
            break;
        case 18:
            tft_->setCursorInPixel(10, 6 * TFT_BIG_CHAR_HEIGHT + 3);
            tft_->setCharColor(COLOR_GRAY);
            tft_->printSmallChars("a,d");
            tft_->setCursor(4, 6);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("Name    : ");
            displayButtonParam(editControl_, BUTTON_PARAM_NAME);
            break;
        case 17:
            tft_->setCursorInPixel(10, 8 * TFT_BIG_CHAR_HEIGHT + 3);
            tft_->setCharColor(COLOR_GRAY);
            tft_->printSmallChar('b');
            tft_->setCursor(4, 8);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("Midi ch : ");
            displayButtonParam(editControl_, BUTTON_PARAM_MIDI_CHANNEL);
            break;
        case 16:
            tft_->setCursorInPixel(10, 9 * TFT_BIG_CHAR_HEIGHT + 3);
            tft_->setCharColor(COLOR_GRAY);
            tft_->printSmallChar('e');

            tft_->setCursor(4, 9);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("CC      : ");
            displayButtonParam(editControl_, BUTTON_PARAM_CONTROLLER);
            break;
        case 15:
            tft_->setCursorInPixel(10, 11 * TFT_BIG_CHAR_HEIGHT + 3);
            tft_->setCharColor(COLOR_GRAY);
            tft_->printSmallChar('c');

            tft_->setCursor(4, 11);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("Type    : ");
            displayButtonParam(editControl_, BUTTON_PARAM_BUTTON_TYPE);
            break;
        case 14:
            tft_->setCursorInPixel(10, 12 * TFT_BIG_CHAR_HEIGHT + 3);
            tft_->setCursor(1, 12);
            tft_->setCharColor(COLOR_GRAY);
            tft_->printSmallChar('f');

            tft_->setCursor(4, 12);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("High    : ");
            displayButtonParam(editControl_, BUTTON_PARAM_HIGH);
            break;
        case 13:
            tft_->setCursorInPixel(10, 13 * TFT_BIG_CHAR_HEIGHT + 3);
            tft_->setCharColor(COLOR_GRAY);
            tft_->printSmallChars("+/-");

            tft_->setCursor(4, 13);
            tft_->setCharColor(COLOR_WHITE);
            tft_->print("Low     : ");
            displayButtonParam(editControl_, BUTTON_PARAM_LOW);
            break;
        case 12:
            refreshStatus_ = 0;
            break;
    }


    if (refreshStatus_ > 0) {
        refreshStatus_--;
    }

    if (refreshStatus_ == 0) {
        tft_->restartRefreshTft();
    }
}

void FMDisplayMidiController::refreshAllScreenByStepSave() {
    switch (refreshStatus_) {
        case 20:
            tft_->fillArea(30, 50, 190, 240, COLOR_RED);
            tft_->fillArea(33, 53, 184, 234, COLOR_BLACK);
        break;
        case 19:
            tft_->setCharColor(COLOR_RED);
            tft_->setCursor(9, 6);
            tft_->print("Save");
        break;
        case 18:
            tft_->setCharColor(COLOR_WHITE);
            tft_->setCursorInPixel(50, 180);
            tft_->printSmallChars("'SEQ' again to save");
            tft_->setCursorInPixel(60, 210);
            tft_->printSmallChars("Any other button");
            tft_->setCursorInPixel(88, 220);
            tft_->printSmallChars("To Cancel");
        break;
        case 17:
            refreshStatus_ = 0;
        break;
    }
    if (refreshStatus_ > 0) {
        refreshStatus_--;
    }
    if (refreshStatus_ == 0) {
        tft_->restartRefreshTft();
    }
}


void FMDisplayMidiController::displayEncoderValue(int encoderNumber) {
    MidiEncoder *encoder = midiControllerState_->getEncoder(pageNumber_, encoderNumber);
    uint8_t x = (encoderNumber > 2 ? (encoderNumber - 3) * 7 : encoderNumber * 7) * TFT_BIG_CHAR_WIDTH + X_OFFSET;
    uint8_t y = (encoderNumber > 2 ? 6 : 3) * TFT_BIG_CHAR_HEIGHT;

    tft_->setCharColor(COLOR_YELLOW);
    tft_->setCursorInPixel(x, y + TFT_BIG_CHAR_HEIGHT + TFT_SMALL_CHAR_HEIGHT);
    if (encoder->value < 10) {
        tft_->print("  ");
    } else if (encoder->value < 100) {
        tft_->print(' ');
    }
    tft_->print(encoder->value);
}

void FMDisplayMidiController::displayButtonValue(int buttonNumber) {
    MidiButton *button = midiControllerState_->getButton(pageNumber_, buttonNumber);
    uint8_t x = (buttonNumber > 2 ? (buttonNumber - 3) * 7 : buttonNumber * 7) * TFT_BIG_CHAR_WIDTH + X_OFFSET;
    uint16_t y = (buttonNumber > 2 ? 13 : 10) * TFT_BIG_CHAR_HEIGHT  + TFT_BIG_CHAR_HEIGHT + TFT_SMALL_CHAR_HEIGHT ;
    uint8_t width = 55;

    if (button->value > 0) {
        tft_->fillArea(x, y, width, 22, COLOR_GREEN);
        tft_->setCharBackgroundColor(COLOR_GREEN);
        tft_->setCharColor(COLOR_BLACK);
    } else {
        if (button->buttonType == MIDI_BUTTON_TYPE_PUSH) {
            tft_->fillArea(x, y, width, 22, COLOR_DARK_GRAY);
        } else {
            tft_->fillArea(x, y, width, 22, COLOR_GREEN);
        }
        tft_->fillArea(x + 1, y + 1, width - 2, 20, COLOR_BLACK);
        tft_->setCharColor(COLOR_GRAY);
    }

    int len = 16;
    if (button->getValue() < 10) {
        len = 6;
    } else if (button->getValue() < 100) {
        len = 11;
    }
    tft_->setCursorInPixel(x + (width >> 1) - len, y + 2);
    tft_->print(button->getValue());
    tft_->setCharBackgroundColor(COLOR_BLACK);
}

void FMDisplayMidiController::encoderTurned(int encoderNumber, int ticks) {
    switch (displayMode_) {
        case DISPLAY_MIDI_CONTROLLER_MAIN: {
            if (!menuPressed_) {
                midiControllerState_->encoderDelta(pageNumber_, midiChannel_, encoderNumber, ticks);
                displayEncoderValue(encoderNumber);
            } else  {
                if (displayMode_ != DISPLAY_MIDI_CONTROLLER_EDIT_ENCODER) {
                    displayMode_ = DISPLAY_MIDI_CONTROLLER_EDIT_ENCODER;
                    editControl_ = encoderNumber;
                    setResetRefreshStatus();
                }
            }
            break;
        }
        case DISPLAY_MIDI_CONTROLLER_EDIT_ENCODER: {
            MidiEncoder* encoder = midiControllerState_->getEncoder(pageNumber_, editControl_);
            switch (encoderNumber) {
            case 0: {
                int newValue = editLetterPosition_ + (ticks > 0 ? 1 : -1);
                newValue = newValue < 0 ? 0 : newValue;
                newValue = newValue > 4 ? 4 : newValue;
                if (editLetterPosition_ != newValue) {
                    editLetterPosition_ = newValue;
                    displayEncoderParam(editControl_, ENCODER_PARAM_NAME);
                }
                break;
            }
            case 3: {
                int newValue = encoder->name[editLetterPosition_] + ticks;
                newValue = newValue < 32 ? 32 : newValue;
                newValue = newValue > 127 ? 127 : newValue;
                if (encoder->name[editLetterPosition_] != newValue) {
                    encoder->name[editLetterPosition_] = newValue;
                    displayEncoderParam(editControl_, ENCODER_PARAM_NAME);
                }
                break;
            }
            case 1: {
                int newValue = encoder->midiChannel + (ticks > 0 ? 1 : -1);
                newValue = newValue < 0 ? 0 : newValue;
                // 16 is valid : it means globalMidiChannel
                newValue = newValue > 16 ? 16 : newValue;
                if (encoder->midiChannel != newValue) {
                    encoder->midiChannel = newValue;
                    displayEncoderParam(editControl_, ENCODER_PARAM_MIDI_CHANNEL);
                }
                break;
            }
            case 4: {
                int newValue = encoder->controller + ticks;
                newValue = newValue < 0 ? 0 : newValue;
                newValue = newValue > 127 ? 127 : newValue;
                if (encoder->controller != newValue) {
                    encoder->controller = newValue;
                    displayEncoderParam(editControl_, ENCODER_PARAM_CONTROLLER);
                }
                break;
            }
            case 2: {
                int newValue = encoder->minValue + ticks;
                newValue = newValue < 0 ? 0 : newValue;
                newValue = newValue > encoder->maxValue ? encoder->maxValue : newValue;
                if (encoder->minValue != newValue) {
                    encoder->minValue = newValue;
                    displayEncoderParam(editControl_, ENCODER_PARAM_MIN);
                }
                break;
            }
            case 5: {
                int newValue = encoder->maxValue + ticks;
                newValue = newValue < encoder->minValue ? encoder->minValue : newValue;
                newValue = newValue > 127 ? 127 : newValue;
                if (encoder->maxValue != newValue) {
                    encoder->maxValue = newValue;
                    displayEncoderParam(editControl_, ENCODER_PARAM_MAX);
                }
                break;
            }
            }
            break;
        }
        case DISPLAY_MIDI_CONTROLLER_EDIT_BUTTON: {
            MidiButton* button = midiControllerState_->getButton(pageNumber_, editControl_);

            switch (encoderNumber) {
            case 0: {
                int newValue = editLetterPosition_ + (ticks > 0 ? 1 : -1);
                newValue = newValue < 0 ? 0 : newValue;
                newValue = newValue > 4 ? 4 : newValue;
                if (editLetterPosition_ != newValue) {
                    editLetterPosition_ = newValue;
                    displayButtonParam(editControl_, BUTTON_PARAM_NAME);
                }
                break;
            }
            case 3: {
                int newValue = button->name[editLetterPosition_] + ticks;
                newValue = newValue < 32 ? 32 : newValue;
                newValue = newValue > 127 ? 127 : newValue;
                if (button->name[editLetterPosition_] != newValue) {
                    button->name[editLetterPosition_] = newValue;
                    displayButtonParam(editControl_, BUTTON_PARAM_NAME);
                }
                break;
            }
            case 1: {
                int newValue = button->midiChannel + (ticks > 0 ? 1 : -1);
                newValue = newValue < 0 ? 0 : newValue;
                // 16 is valid : it means globalMidiChannel
                newValue = newValue > 16 ? 16 : newValue;
                if (button->midiChannel != newValue) {
                    button->midiChannel = newValue;
                    displayButtonParam(editControl_, BUTTON_PARAM_MIDI_CHANNEL);
                }
                break;
            }
            case 4: {
                int newValue = button->controller + ticks;
                newValue = newValue < 0 ? 0 : newValue;
                newValue = newValue > 127 ? 127 : newValue;
                if (button->controller != newValue) {
                    button->controller = newValue;
                    displayButtonParam(editControl_, BUTTON_PARAM_CONTROLLER);
                }
                break;
            }
            case 2: {
                MidiButtonType newValue = ticks < 0 ? MIDI_BUTTON_TYPE_PUSH : MIDI_BUTTON_TYPE_TOGGLE;
                if (button->buttonType != newValue) {
                    button->buttonType = newValue;
                    displayButtonParam(editControl_, BUTTON_PARAM_BUTTON_TYPE);
                }
                break;
            }
            case 5: {
                int newValue = button->valueHigh + ticks;
                newValue = newValue < (button->valueLow + 1) ? (button->valueLow + 1)  : newValue;
                newValue = newValue > 127 ? 127 : newValue;
                if (button->valueHigh != newValue) {
                    button->valueHigh = newValue;
                    displayButtonParam(editControl_, BUTTON_PARAM_HIGH);
                }
                break;
            }
            }

            break;
        }
    }
}


void FMDisplayMidiController::displayMidiChannel() {
    tft_->setCharColor(COLOR_DARK_GRAY);
    tft_->setCursorInPixel(10, 28);
    tft_->printSmallChars("Midi Channel ");
    tft_->setCharColor(COLOR_WHITE);
    tft_->printSmallChar(midiChannel_ + 1);
    if ((midiChannel_ + 1) < 10) {
        tft_->printSmallChar(' ');
    }
    tft_->setCharColor(COLOR_DARK_GRAY);
    tft_->setCursorInPixel(178, 28);
    tft_->printSmallChars("page ");
    tft_->setCharColor(COLOR_WHITE);
    tft_->printSmallChar(pageNumber_ + 1);

}


void FMDisplayMidiController::displayEncoderParam(int encoderNumber, EncoderParamType paramType) {
    MidiEncoder* encoder = midiControllerState_->getEncoder(pageNumber_, encoderNumber);

    switch (paramType) {
    case ENCODER_PARAM_NAME:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 6);
        tft_->print(encoder->name);
        tft_->setCursor(14 + editLetterPosition_, 6);
        tft_->setCharColor(COLOR_RED);
        if (encoder->name[editLetterPosition_] == ' ') {
            tft_->print('_');
        } else {
            tft_->print(encoder->name[editLetterPosition_]);
        }
        break;
    case ENCODER_PARAM_MIDI_CHANNEL:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 8);
        if (encoder->midiChannel == 16) {
            tft_->print("--");
        } else {
            tft_->print(encoder->midiChannel + 1);
            if ((encoder->midiChannel +1) < 10) {
                tft_->print(' ');
            }
        }
        break;
    case ENCODER_PARAM_CONTROLLER:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 9);
        tft_->print(encoder->controller);
        if (encoder->controller < 100) {
            tft_->print(' ');
            if (encoder->controller < 10) {
                tft_->print(' ');
            }
        }
        break;
    case ENCODER_PARAM_MIN:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 11);
        tft_->print(encoder->minValue);
        if (encoder->minValue < 100) {
            tft_->print(' ');
            if (encoder->minValue < 10) {
                tft_->print(' ');
            }
        }
        break;
    case ENCODER_PARAM_MAX:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 12);
        tft_->print(encoder->maxValue);
        if (encoder->maxValue < 100) {
            tft_->print(' ');
            if (encoder->maxValue < 10) {
                tft_->print(' ');
            }
        }
        break;
    }

}


void FMDisplayMidiController::displayButtonParam(int buttonNumber, ButtonParamType paramType) {
    MidiButton* button = midiControllerState_->getButton(pageNumber_, buttonNumber);

    switch (paramType) {
    case BUTTON_PARAM_NAME:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 6);
        tft_->print(button->name);
        tft_->setCursor(14 + editLetterPosition_, 6);
        tft_->setCharColor(COLOR_RED);
        if (button->name[editLetterPosition_] == ' ') {
            tft_->print('_');
        } else {
            tft_->print(button->name[editLetterPosition_]);
        }
        break;
    case BUTTON_PARAM_MIDI_CHANNEL:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 8);
        if (button->midiChannel == 16) {
            tft_->print("--");
        } else {
            tft_->print(button->midiChannel + 1);
            if ((button->midiChannel +1) < 10) {
                tft_->print(' ');
            }
        }
        break;
    case BUTTON_PARAM_CONTROLLER:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 9);
        tft_->print(button->controller);
        if (button->controller < 100) {
            tft_->print(' ');
            if (button->controller < 10) {
                tft_->print(' ');
            }
        }
        break;
    case BUTTON_PARAM_BUTTON_TYPE:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 11);
        if (button->buttonType == MIDI_BUTTON_TYPE_PUSH) {
            tft_->print("Push");
        } else {
            tft_->print("Togl");
        }
        break;
    case BUTTON_PARAM_HIGH:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 12);
        tft_->print(button->valueHigh);
        if (button->valueHigh < 100) {
            tft_->print(' ');
            if (button->valueHigh < 10) {
                tft_->print(' ');
            }
        }
        break;
    case BUTTON_PARAM_LOW:
        tft_->setCharColor(COLOR_YELLOW);
        tft_->setCursor(14, 13);
        tft_->print(button->valueLow);
        if (button->valueHigh < 100) {
            tft_->print(' ');
            if (button->valueLow < 10) {
                tft_->print(' ');
            }
        }
        break;
    }

}



void FMDisplayMidiController::buttonUp(int buttonNumber) {
    if (displayMode_ == DISPLAY_MIDI_CONTROLLER_SAVE) {
        return;
    } else if (displayMode_ == DISPLAY_MIDI_CONTROLLER_MAIN) {
        if (buttonNumber >= BUTTON_PFM3_1 && buttonNumber <= BUTTON_PFM3_6) {
            if (midiControllerState_->buttonUp(pageNumber_, midiChannel_, buttonNumber)) {
                displayButtonValue(buttonNumber);
            }
        } else if (buttonNumber == BUTTON_PFM3_MENU) {
            menuPressed_ = false;
        }
    }
}

void FMDisplayMidiController::buttonDown(int buttonNumber) {
    if (displayMode_ == DISPLAY_MIDI_CONTROLLER_SAVE) {
        if (buttonNumber == BUTTON_PFM3_SEQUENCER) {
            tft_->fillArea(30, 50, 190, 240, COLOR_RED);
            tft_->setCharBackgroundColor(COLOR_RED);
            tft_->setCharColor(COLOR_BLACK);
            tft_->setCursorInPixel(94, 120);
            tft_->print("Saved");
            tft_->setCharBackgroundColor(COLOR_BLACK);
            midiControllerFile_->saveConfig(midiControllerState_);
            HAL_Delay(1500);
        }
        displayMode_ = DISPLAY_MIDI_CONTROLLER_MAIN;
        setResetRefreshStatus();
        return;
    } else if (displayMode_ == DISPLAY_MIDI_CONTROLLER_MAIN) {
        if (buttonNumber >= BUTTON_PFM3_1 && buttonNumber <= BUTTON_PFM3_6) {
            if (!menuPressed_) {
                midiControllerState_->buttonDown(pageNumber_, midiChannel_, buttonNumber);
                displayButtonValue(buttonNumber);
            } else  {
                displayMode_ = DISPLAY_MIDI_CONTROLLER_EDIT_BUTTON;
                editControl_ = buttonNumber;
                setResetRefreshStatus();
            }
        } else if (buttonNumber == BUTTON_PFM3_MENU) {
            menuPressed_ = true;
        } else if (buttonNumber == BUTTON_PFM3_SEQUENCER) {
            displayMode_ = DISPLAY_MIDI_CONTROLLER_SAVE;
            refreshStatus_ = 20;
            HAL_Delay(100);
        } else if (buttonNumber == BUTTON_NEXT_INSTRUMENT) {
            if (!menuPressed_) {
                if (pageNumber_ < (MIDI_NUMBER_OF_PAGES -1)) {
                    pageNumber_ ++;
                    setResetRefreshStatus();
                }
            } else {
                if (midiChannel_ < 15) {
                    midiChannel_ ++;
                    displayMidiChannel();
                }
            }
        } else if (buttonNumber == BUTTON_PREVIOUS_INSTRUMENT) {
            if (!menuPressed_) {
                if (pageNumber_ > 0) {
                    pageNumber_ --;
                    setResetRefreshStatus();
                }
            } else {
                if (midiChannel_ > 0) {
                    midiChannel_ --;
                    displayMidiChannel();
                }
            }
        }
    } else {
        if (displayMode_ == DISPLAY_MIDI_CONTROLLER_EDIT_BUTTON) {
            MidiButton* button = midiControllerState_->getButton(pageNumber_, editControl_);
            if (buttonNumber == BUTTON_PREVIOUS_INSTRUMENT && button->valueLow > 0) {
                button->valueLow--;
                displayButtonParam(editControl_, BUTTON_PARAM_LOW);
            } else if (buttonNumber == BUTTON_NEXT_INSTRUMENT && button->valueLow < (button->valueHigh -1)) {
                button->valueLow++;
                displayButtonParam(editControl_, BUTTON_PARAM_LOW);
            }
        }
        if (buttonNumber == BUTTON_PFM3_MENU) {
            displayMode_ = DISPLAY_MIDI_CONTROLLER_MAIN;
            setResetRefreshStatus();
        }
    }
}


// USELESS
void FMDisplayMidiController::encoderTurnedWhileButtonPressed(int encoder, int ticks, int buttonNumber) {
}

void FMDisplayMidiController::buttonPressed(int buttonNumber) {
}

void FMDisplayMidiController::buttonLongPressed(int button) {
}

void FMDisplayMidiController::twoButtonsPressed(int button1, int button2) {
}
