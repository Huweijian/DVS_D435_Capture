#ifndef EVENTHANDLER_H
#define EVENTHANDLER_H

#include <vector>
#include <functional>

/**
 * Convenience class for handling callback events. Inspired by the C# EventHandler.
 */
template<typename... TParamTypes>
class EventHandler
{
public:
    /**
     * @brief Add callback function to handler.
     * @param _callback
     * @return
     */
    EventHandler& operator += (const std::function<void(TParamTypes...)>& _callback)
    {
        callbacks_.push_back(_callback);
        return *this;
    }

    /**
     * @brief Invoke callbacks.
     * @param _parameters
     */
    void operator () (TParamTypes... _parameters)
    {
        for(auto& callback : callbacks_)
            callback(_parameters...);
    }

private:
    std::vector<std::function<void(TParamTypes...)>> callbacks_;
};

#endif // EVENTHANDLER_H
