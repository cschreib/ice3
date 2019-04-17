#include "application.hpp"
#include "world.hpp"
#include <system_error>

int main(int argc, char* argv[])
{
    application app;

    try
    {
        app.read_config();
        app.push_state<world>(true, true);
        app.start();
    }
    catch (utils::exception& e)
    {
        std::cout << "# Error # : " << e.get_description() << std::endl;
        return 1;
    }
    catch (std::system_error& e)
    {
        switch ((std::errc)e.code().value())
        {
        case std::errc::resource_deadlock_would_occur :
            std::cout << "std::system_error::resource deadlock : " << e.what() << std::endl;
            break;
        case std::errc::no_such_process :
            std::cout << "std::system_error::no such process : " << e.what() << std::endl;
            break;
        case std::errc::invalid_argument :
            std::cout << "std::system_error::invalid argument : " << e.what() << std::endl;
            break;
        default :
            std::cout << "std::system_error : " << e.what() << std::endl;
            break;
        }
        return 1;
    }

    return 0;
}
