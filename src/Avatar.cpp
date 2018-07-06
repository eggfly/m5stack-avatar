// Copyright (c) Shinya Ishikawa. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full
// license information.

#include "Avatar.h"
namespace m5avatar {
const uint32_t DEFAULT_STACK_SIZE = 4096;

unsigned int seed = 0;

// TODO(meganetaaan): make read-only
DriveContext::DriveContext(Avatar *avatar) : avatar{avatar} {}

Avatar *DriveContext::getAvatar() { return avatar; }

void updateBreath(void *args) {
  int c = 0;
  DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
  for (;;) {
    c = c + 1 % 100;
    float f = sin(c * 2 * PI / 100.0);
    ctx->getAvatar()->setBreath(f);
    delay(33);
  }
}

void drawLoop(void *args) {
  DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
  Avatar *avatar = ctx->getAvatar();
  for (;;) {
    if (avatar->isDrawing()) {
      avatar->draw();
    }
    delay(33);
  }
}

void saccade(void *args) {
  DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
  for (;;) {
    float vertical = rand_r(&seed) / (RAND_MAX / 2.0) - 1;
    float horizontal = rand_r(&seed) / (RAND_MAX / 2.0) - 1;
    ctx->getAvatar()->setGaze(vertical, horizontal);
    delay(500 + 100 * random(20));
  }
}

void blink(void *args) {
  DriveContext *ctx = reinterpret_cast<DriveContext *>(args);
  for (;;) {
    ctx->getAvatar()->setEyeOpenRatio(1);
    delay(2500 + 100 * random(20));
    ctx->getAvatar()->setEyeOpenRatio(0);
    delay(300 + 10 * random(20));
  }
}

Avatar::Avatar() : Avatar(new Face()) {}

Avatar::Avatar(Face *face)
    : face{face},
      _isDrawing{true},
      expression{Expression::Neutral},
      breath{0},
      eyeOpenRatio{1},
      mouthOpenRatio{0},
      gazeV{0},
      gazeH{0},
      palette{ColorPalette()},
      speechText{""} {}

void Avatar::setFace(Face *face) { this->face = face; }

Face *Avatar::getFace() const { return face; }

void Avatar::addTask(TaskFunction_t f, const char* name) {
  DriveContext *ctx = new DriveContext(this);
  // TODO(meganetaaan): set a task handler
  xTaskCreatePinnedToCore(f, /* Function to implement the task */
                          name, /* Name of the task */
                          DEFAULT_STACK_SIZE, /* Stack size in words */
                          ctx,                /* Task input parameter */
                          1,                  /* P2014riority of the task */
                          NULL,               /* Task handle. */
                          1); /* Core where the task should run */
}

void Avatar::init() {
  DriveContext *ctx = new DriveContext(this);
  // TODO(meganetaaan): keep handle of these tasks
  xTaskCreatePinnedToCore(drawLoop,     /* Function to implement the task */
                          "drawLoop",   /* Name of the task */
                          4096,         /* Stack size in words */
                          ctx,          /* Task input parameter */
                          1,            /* Priority of the task */
                          NULL,         /* Task handle. */
                          1);           /* Core where the task should run */
  xTaskCreatePinnedToCore(saccade,      /* Function to implement the task */
                          "saccade",    /* Name of the task */
                          4096,         /* Stack size in words */
                          ctx,          /* Task input parameter */
                          3,            /* Priority of the task */
                          NULL,         /* Task handle. */
                          1);           /* Core where the task should run */
  xTaskCreatePinnedToCore(updateBreath, /* Function to implement the task */
                          "breath",     /* Name of the task */
                          4096,         /* Stack size in words */
                          ctx,          /* Task input parameter */
                          2,            /* Priority of the task */
                          NULL,         /* Task handle. */
                          1);           /* Core where the task should run */
  xTaskCreatePinnedToCore(blink,        /* Function to implement the task */
                          "blink",      /* Name of the task */
                          4096,         /* Stack size in words */
                          ctx,          /* Task input parameter */
                          2,            /* Priority of the task */
                          NULL,         /* Task handle. */
                          1);           /* Core where the task should run */
}

void Avatar::stop() { _isDrawing = false; }

void Avatar::start() { _isDrawing = true; }

void Avatar::draw() {
  Gaze g = Gaze(this->gazeV, this->gazeH);
  DrawContext *ctx = new DrawContext(this->expression, this->breath,
                                     this->palette, g, this->eyeOpenRatio,
                                     this->mouthOpenRatio, this->speechText);
  face->draw(ctx);
  delete ctx;
}

bool Avatar::isDrawing() { return _isDrawing; }

void Avatar::setExpression(Expression expression) {
  this->expression = expression;
}

void Avatar::setBreath(float breath) { this->breath = breath; }

void Avatar::setColorPalette(ColorPalette cp) { palette = cp; }

ColorPalette Avatar::getColorPalette(void) const { return this->palette; }

void Avatar::setMouthOpenRatio(float ratio) { this->mouthOpenRatio = ratio; }

void Avatar::setEyeOpenRatio(float ratio) { this->eyeOpenRatio = ratio; }

void Avatar::setGaze(float vertical, float horizontal) {
  this->gazeV = vertical;
  this->gazeH = horizontal;
}

void Avatar::setSpeechText(const char *speechText) {
  this->speechText = speechText;
}

}  // namespace m5avatar
