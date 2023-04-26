#include "bililive/bililive/ui/bililive_command_receiver.h"

BililiveCommandReceiver::BililiveCommandReceiver()
    : command_controller_(std::make_unique<bililive::BililiveCommandController>(this))
{}

BililiveCommandReceiver::~BililiveCommandReceiver()
{}
