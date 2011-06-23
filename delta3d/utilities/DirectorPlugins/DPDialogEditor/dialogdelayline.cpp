#include <dialogdelayline.h>

#include <QtGui/QDoubleSpinBox>

#include <dtDirector/node.h>


///////////////////////////////////////////////////////////////////////////////
DialogDelayLineType::DialogDelayLineType()
   : mDelayTime(1.0f)
{
}

///////////////////////////////////////////////////////////////////////////////
DialogDelayLineType::~DialogDelayLineType()
{
}

////////////////////////////////////////////////////////////////////////////////
DialogLineType* DialogDelayLineType::Create() const
{
   return new DialogDelayLineType();
}

////////////////////////////////////////////////////////////////////////////////
void DialogDelayLineType::GenerateNode(DialogLineItem* line, dtDirector::Node* prevNode, const std::string& output, DirectorDialogEditorPlugin* editor)
{
   dtDirector::Node* delayNode = editor->CreateNode("Delay", "General", prevNode, 200);
   delayNode->SetFloat(mDelayTime, "Delay");
   editor->Connect(prevNode, delayNode, output, "Start");

   GenerateNodeForReferences(line, delayNode, "Start", editor);

   GenerateNodeForChild(line->GetChildLine(), delayNode, "Time Elapsed", editor);
}

////////////////////////////////////////////////////////////////////////////////
bool DialogDelayLineType::ShouldOperateOn(const dtDirector::Node* node) const
{
   if (node && node->GetType().GetFullName() == "General.Delay")
   {
      return true;
   }

   return false;
}

////////////////////////////////////////////////////////////////////////////////
void DialogDelayLineType::OperateOn(DialogLineItem* line, dtDirector::Node* node, DirectorDialogEditorPlugin* editor)
{
   CheckForReferencing(line, node, "Start", editor);

   mDelayTime = node->GetFloat("Delay");

   OperateOnChild(line, node, "Time Elapsed", editor);
}

////////////////////////////////////////////////////////////////////////////////
QString DialogDelayLineType::GetDisplayName(const DialogTreeWidget* tree) const
{
   return QString::number(mDelayTime) + " Seconds";
}

////////////////////////////////////////////////////////////////////////////////
QWidget* DialogDelayLineType::CreateInlineEditor(QWidget* parent, DialogTreeWidget* tree) const
{
   return new QDoubleSpinBox(parent);
}

////////////////////////////////////////////////////////////////////////////////
void DialogDelayLineType::RefreshInlineEditor(QWidget* editor) const
{
   QDoubleSpinBox* control = dynamic_cast<QDoubleSpinBox*>(editor);
   if (control)
   {
      control->setValue((double)mDelayTime);
   }
}

////////////////////////////////////////////////////////////////////////////////
void DialogDelayLineType::RefreshInlineData(QWidget* editor)
{
   QDoubleSpinBox* control = dynamic_cast<QDoubleSpinBox*>(editor);
   if (control)
   {
      mDelayTime = (float)control->value();
   }
}

///////////////////////////////////////////////////////////////////////////////