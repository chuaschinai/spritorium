#include "task.hpp"

#include "app/app_state.hpp"
#include "app/logger.hpp"

namespace spto
{
    void Task_DestroyView::Execute()
    {
        const auto it = std::remove_if(g.Views.begin(), g.Views.end(),
        [this](const auto& el)
        {
            return el->Id == Id;
        });
        
        if (it == g.Views.end())
        {
            spto::Warn("View with Id: {}, not found to be destroyed!", Id);
            return;
        }

        g.Views.erase(it);
        g.SelectedView = (g.Views.size() > 0 ? g.Views.size() - 1 : -1);
    }

}