#include "internal_sys_mock.hpp"

#include <ipmiblob/ipmi_errors.hpp>
#include <ipmiblob/ipmi_handler.hpp>

namespace ipmiblob
{

using ::testing::_;
using ::testing::Return;

TEST(IpmiHandlerTest, OpenAllFails)
{
    /* Open against all device files fail. */
    std::unique_ptr<internal::InternalSysMock> sysMock =
        std::make_unique<internal::InternalSysMock>();
    EXPECT_CALL(*sysMock, open(_, _)).WillRepeatedly(Return(-1));

    IpmiHandler ipmi(std::move(sysMock));
    EXPECT_THROW(ipmi.open(), IpmiException);
}

} // namespace ipmiblob
