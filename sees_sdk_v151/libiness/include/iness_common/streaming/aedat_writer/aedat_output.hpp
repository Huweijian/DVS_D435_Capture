#ifndef AEDAT_OUTPUT_HPP
#define AEDAT_OUTPUT_HPP

#include <memory>

#include <iness_common/streaming/aedat_writer/aedat_source_info.hpp>
#include <iness_common/types/events/i_event_packet.hpp>
#include <iness_common/types/generated/aedat_type_generated.h>

namespace iness
{
namespace aedat
{

/**
 * @brief Handles Aedat packet recording to a file. The class is thread safe.
 */
class AedatOutput
{
public:
    AedatOutput();
    ~AedatOutput();

    /**
     * @brief Start the recording.
     * @param _source_info Description of the source that will write to the file.
     * @param _filename File to which the data is written.
     * @param _types_to_record Only record specific packet types. If empty all the packets will be written to the file.
     */
    void start(const AedatSourceInfo& _source_info,
               const std::string _filename,
               const std::vector<iness::serialization::AedatType>& _types_to_record = std::vector<iness::serialization::AedatType>());

    /**
     * @return True if start was called before.
     */
    bool isRecording();

    /**
     * @brief Writes packet to file. this->start(...) needs to be called before to open the file.
     * @param _packet
     */
    void writePacket(const std::shared_ptr<IEventPacket> _packet);
    
    /**
     * @brief Check if no errors happend so far while writing to file.
     * It can also be checked after stop() is called.
     * The state is reset only when start() is called.
     * @return
     */
    bool isOk();

    /**
     * @brief Stops the recording.
     */
    std::string stop();

private:
    struct Implementation; //!< Forward Declaration
    std::unique_ptr<Implementation> impl_; //!< Pointer to implementation
};

}
}

#endif // DATA_OUTPUT_HPP
