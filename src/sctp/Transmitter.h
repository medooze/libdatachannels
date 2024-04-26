#ifndef SCTP_TRANSMITTER_H
#define SCTP_TRANSMITTER_H

#include "Chunk.h"

#include <vector>
#include <memory>

namespace sctp
{

class Transmitter
{
public:
	virtual ~Transmitter() = default;
	virtual void Enqueue(const std::shared_ptr<Chunk>& chunk) = 0;
	virtual void Enqueue(const std::vector<std::shared_ptr<Chunk>>& chunkBundle) = 0;
};

}

#endif