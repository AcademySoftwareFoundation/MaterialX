//
// TM & (c) 2019 Lucasfilm Entertainment Company Ltd. and Lucasfilm Ltd.
// All rights reserved.  See LICENSE.txt for license.
//

#include <MaterialXRuntime/Private/PvtTraversal.h>
#include <MaterialXRuntime/Private/PvtElement.h>

namespace MaterialX
{

PvtStageIterator::PvtStageIterator() :
    _current(nullptr),
    _filter(nullptr)
{
}

PvtStageIterator::PvtStageIterator(PvtDataHandle root, RtTraversalFilter filter) :
    _current(nullptr),
    _filter(filter)
{
    if (root->hasApi(RtApiType::STAGE))
    {
        // Initialize the stack and start iteration to the first element.
        PvtStage* stage = root->asA<PvtStage>();
        _stack.push_back(std::make_tuple(stage, -1, -1));
        ++*this;
    }
}

PvtStageIterator::PvtStageIterator(const PvtStageIterator& other) :
    _current(other._current),
    _stack(other._stack),
    _filter(other._filter)
{
}

PvtStageIterator& PvtStageIterator::operator++()
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
        PvtStage* stage = std::get<0>(frame);
        int& elemIndex = std::get<1>(frame);
        int& stageIndex = std::get<2>(frame);

        bool pop = true;

        if (elemIndex + 1 < int(stage->getChildren().size()))
        {
            _current = stage->getChildren()[++elemIndex];
            if (!_filter || _filter(PvtObject::object(_current)))
            {
                return *this;
            }
            pop = false;
        }
        else if (stageIndex + 1 < int(stage->getReferencedStages().size()))
        {
            PvtStage* refStage = stage->getReferencedStages()[++stageIndex]->asA<PvtStage>();
            if (!refStage->getChildren().empty())
            {
                _stack.push_back(std::make_tuple(refStage, 0, stageIndex));
                _current = refStage->getChildren()[0];
                if (!_filter || _filter(PvtObject::object(_current)))
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


PvtTreeIterator::PvtTreeIterator() :
    _current(nullptr),
    _filter(nullptr)
{
}

PvtTreeIterator::PvtTreeIterator(PvtDataHandle root, RtTraversalFilter filter) :
    _current(nullptr),
    _filter(filter)
{
    if (root->hasApi(RtApiType::ELEMENT))
    {
        // Initialize the stack and start iteration to the first element.
        PvtElement* elem = root->asA<PvtElement>();
        _stack.push_back(std::make_tuple(elem, -1, -1));
        ++*this;
    }
}

PvtTreeIterator::PvtTreeIterator(const PvtTreeIterator& other) :
    _current(other._current),
    _stack(other._stack),
    _filter(other._filter)
{
}

PvtTreeIterator& PvtTreeIterator::operator++()
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
            PvtElement* elem = _current->asA<PvtElement>();
            if (elem->numChildren())
            {
                _stack.push_back(std::make_tuple(elem, 0, -1));
                _current = elem->getChild(0);
                if (!_filter || _filter(PvtObject::object(_current)))
                {
                    return *this;
                }
            }
        }

        StackFrame& frame = _stack.back();
        PvtElement* elem = std::get<0>(frame);
        int& elemIndex = std::get<1>(frame);
        int& stageIndex = std::get<2>(frame);

        bool filterUsed = false;

        if (elemIndex + 1 < int(elem->getChildren().size()))
        {
            _current = elem->getChildren()[++elemIndex];
            if (!_filter || _filter(PvtObject::object(_current)))
            {
                return *this;
            }
            filterUsed = true;
        }
        else if (elem->hasApi(RtApiType::STAGE))
        {
            PvtStage* stage = elem->asA<PvtStage>();
            if (stageIndex + 1 < int(stage->getReferencedStages().size()))
            {
                PvtStage* refStage = stage->getReferencedStages()[++stageIndex]->asA<PvtStage>();
                if (refStage->numChildren())
                {
                    _stack.push_back(std::make_tuple(refStage, 0, stageIndex));
                    _current = refStage->getChild(0);
                    if (!_filter || _filter(PvtObject::object(_current)))
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



PvtGraphIterator::PvtGraphIterator() :
    _filter(nullptr)
{
}

PvtGraphIterator::PvtGraphIterator(RtPort root, RtTraversalFilter filter) :
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

PvtGraphIterator::PvtGraphIterator(const PvtGraphIterator& other) :
    _current(other._current),
    _stack(other._stack),
    _filter(other._filter)
{
}

PvtGraphIterator& PvtGraphIterator::operator++()
{
    if (_current.first.data())
    {
        PvtNode* node = _current.first.data()->asA<PvtNode>();

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
        PvtNode* node = parentFrame.first.data()->asA<PvtNode>();
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

void PvtGraphIterator::extendPathUpstream(const RtPort& upstream, const RtPort& downstream)
{
    // Check for cycles.
    if (_path.count(upstream))
    {
        throw ExceptionRuntimeError("Encountered cycle at element: " + upstream.data()->asA<PvtNode>()->getName().str() + "." + upstream.getName().str());
    }

    // Extend the current path to the new element.
    _path.insert(upstream);
    _current.first = upstream;
    _current.second = downstream;
}

void PvtGraphIterator::returnPathDownstream(const RtPort& upstream)
{
    _path.erase(upstream);
    _current.first = RtPort();
    _current.second = RtPort();
}

}
