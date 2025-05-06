#include "Id.hh"

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace kstd {

Uuid generateUuid() {
    using namespace boost;
    return uuids::to_string(uuids::random_generator()());
}

WithUuid::WithUuid() : m_uuid(generateUuid()) {}

const Uuid& WithUuid::getUuid() const { return m_uuid; }

}  // namespace kstd
