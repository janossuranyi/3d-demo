#include "animation.h"

uint Animation::addSampler()
{
    samplers.emplace_back();
    samplers.back().reset(new Sampler());

    return samplers.size() - 1;
}

uint Animation::addInput()
{
    inputs.emplace_back();

    return inputs.size() - 1;
}

uint Animation::addChannel()
{
    channels.emplace_back();

    return channels.size() - 1;
}
