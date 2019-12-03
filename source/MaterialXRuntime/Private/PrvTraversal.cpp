//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PrvTraversal.h>
#include <MaterialXRuntime/Private/PrvElement.h>

namespace MaterialX
{

PrvStageIterator::PrvStageIterator() :
    _current(nullptr),
    _filter(nullptr)
{
}

PrvStageIterator::PrvStageIterator(PrvObjectHandle root, RtTraversalFilter filter) :
    _current(nullptr),
    _filter(filter)
{
    if (root->hasApi(RtApiType::STAGE))
    {
        // Initialize the stack and start iteration to the first element.
        PrvStage* stage = root->asA<PrvStage>();
        _stack.push_back(std::make_tuple(stage, -1, -1));
        ++*this;
    }
}

PrvStageIterator::PrvStageIterator(const PrvStageIterator& other) :
    _current(other._current),
    _stack(other._stack),
    _filter(other._filter)
{
}

PrvStageIterator& PrvStageIterator::operator++()
{
    while (true)
    {
        if (_stack.empty())
        {
            // Traversal is complete.
            abort();
            return *this;
        }

        StackFrame& frame = _stack.back();
        PrvStage* stage = std::get<0>(frame);
        int& elemIndex = std::get<1>(frame);
        int& stageIndex = std::get<2>(frame);

        bool pop = true;

        if (elemIndex + 1 < int(stage->getChildren().size()))
        {
            _current = stage->getChildren()[++elemIndex];
            if (!_filter || _filter(RtObject(_current)))
            {
                return *this;
            }
            pop = false;
        }
        else if (stageIndex + 1 < int(stage->getReferencedStages().size()))
        {
            PrvStage* refStage = stage->getReferencedStages()[++stageIndex]->asA<PrvStage>();
            if (!refStage->getChildren().empty())
            {
                _stack.push_back(std::make_tuple(refStage, 0, stageIndex));
                _current = refStage->getChildren()[0];
                if (!_filter || _filter(RtObject(_current)))
                {
                    return *this;
                }
                pop = false;
            }
        }

        if (pop)
        {
            _stack.pop_back();
        }
    }
}


PrvTreeIterator::PrvTreeIterator() :
    _current(nullptr),
    _filter(nullptr)
{
}

PrvTreeIterator::PrvTreeIterator(PrvObjectHandle root, RtTraversalFilter filter) :
    _current(nullptr),
    _filter(filter)
{
    if (root->hasApi(RtApiType::ELEMENT))
    {
        // Initialize the stack and start iteration to the first element.
        PrvElement* elem = root->asA<PrvElement>();
        _stack.push_back(std::make_tuple(elem, -1, -1));
        ++*this;
    }
}

PrvTreeIterator::PrvTreeIterator(const PrvTreeIterator& other) :
    _current(other._current),
    _stack(other._stack),
    _filter(other._filter)
{
}

PrvTreeIterator& PrvTreeIterator::operator++()
{
    while (true)
    {
        if (_stack.empty())
        {
            // Traversal is complete.
            abort();
            return *this;
        }

        if (_current && 
            _current->hasApi(RtApiType::ELEMENT) &&
            !_current->hasApi(RtApiType::STAGE))
        {
            PrvElement* elem = _current->asA<PrvElement>();
            if (elem->numChildren())
            {
                _stack.push_back(std::make_tuple(elem, 0, -1));
                _current = elem->getChild(0);
                if (!_filter || _filter(RtObject(_current)))
                {
                    return *this;
                }
            }
        }

        StackFrame& frame = _stack.back();
        PrvElement* elem = std::get<0>(frame);
        int& elemIndex = std::get<1>(frame);
        int& stageIndex = std::get<2>(frame);

        bool filterUsed = false;

        if (elemIndex + 1 < int(elem->getChildren().size()))
        {
            _current = elem->getChildren()[++elemIndex];
            if (!_filter || _filter(RtObject(_current)))
            {
                return *this;
            }
            filterUsed = true;
        }
        else if (elem->hasApi(RtApiType::STAGE))
        {
            PrvStage* stage = elem->asA<PrvStage>();
            if (stageIndex + 1 < int(stage->getReferencedStages().size()))
            {
                PrvStage* refStage = stage->getReferencedStages()[++stageIndex]->asA<PrvStage>();
                if (refStage->numChildren())
                {
                    _stack.push_back(std::make_tuple(refStage, 0, stageIndex));
                    _current = refStage->getChild(0);
                    if (!_filter || _filter(RtObject(_current)))
                    {
                        return *this;
                    }
                    filterUsed = true;
                }
            }
        }

        if (!filterUsed)
        {
            // We got here without the filter being used.
            // So the current stack frame has been completed
            // and we can unroll the stack to previous frame.
            _stack.pop_back();
        }
    }
}



PrvGraphIterator::PrvGraphIterator() :
    _filter(nullptr)
{
}

PrvGraphIterator::PrvGraphIterator(RtPort root, RtTraversalFilter filter) :
    _filter(filter)
{
    if (root.isOutput())
    {
        _current.first = root;
    }
    else
    {
        _current.first = root.getSourcePort();
        _current.second = root;
    }
}

PrvGraphIterator::PrvGraphIterator(const PrvGraphIterator& other) :
    _current(other._current),
    _stack(other._stack),
    _filter(other._filter)
{
}

PrvGraphIterator& PrvGraphIterator::operator++()
{
    if (_current.first.data())
    {
        PrvNode* node = _current.first.data()->asA<PrvNode>();

        // Check if we have any inputs.
        if (node->numInputs())
        {
            // Traverse to the first upstream edge of this element.
            const size_t inputIndex = node->getInputsOffset();
            _stack.push_back(StackFrame(_current.first, inputIndex));

            RtPort input = node->getPort(inputIndex);
            RtPort output = input.getSourcePort();

            if (output)
            {
                extendPathUpstream(output, input);
                return *this;
            }
        }
    }

    while (true)
    {
        if (_current.first.data())
        {
            returnPathDownstream(_current.first);
        }

        if (_stack.empty())
        {
            // Traversal is complete.
            abort();
            return *this;
        }

        // Traverse to our siblings.
        StackFrame& parentFrame = _stack.back();
        PrvNode* node = parentFrame.first.data()->asA<PrvNode>();
        while (parentFrame.second + 1 < node->numPorts())
        {
            RtPort input = node->getPort(++parentFrame.second);
            RtPort output = input.getSourcePort();
            if (output)
            {
                extendPathUpstream(output, input);
                return *this;
            }
        }

        // Traverse to our parent's siblings.
        returnPathDownstream(parentFrame.first);
        _stack.pop_back();
    }

    return *this;
}

void PrvGraphIterator::extendPathUpstream(const RtPort& upstream, const RtPort& downstream)
{
    // Check for cycles.
    if (_path.count(upstream))
    {
        throw ExceptionRuntimeError("Encountered cycle at element: " + upstream.data()->asA<PrvNode>()->getName().str() + "." + upstream.getName().str());
    }

    // Extend the current path to the new element.
    _path.insert(upstream);
    _current.first = upstream;
    _current.second = downstream;
}

void PrvGraphIterator::returnPathDownstream(const RtPort& upstream)
{
    _path.erase(upstream);
    _current.first = RtPort();
    _current.second = RtPort();
}

}
