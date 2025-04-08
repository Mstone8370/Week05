#include "Components/SceneComponent.h"
#include "Level.h"
#include "Math/JungleMath.h"
#include "UObject/ObjectFactory.h"
#include "Engine/Classes/Components/SkySphereComponent.h"
#include "Core/Container/Array.h"

#include "UObject/Casts.h"

#include "TextRenderComponent.h"
#include "GameFramework/Actor.h"

USceneComponent::USceneComponent() :RelativeLocation(FVector(0.f, 0.f, 0.f)), RelativeRotation(FVector(0.f, 0.f, 0.f)), RelativeScale3D(FVector(1.f, 1.f, 1.f))
{
}

USceneComponent::~USceneComponent()
{
}

void USceneComponent::InitializeComponent()
{
    Super::InitializeComponent();

}

void USceneComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}

void USceneComponent::DestroyComponent(bool bPromoteChildren)
{
    TArray<USceneComponent*> ChildrenCopy = AttachChildren;
    for (auto& Child : ChildrenCopy)
    {
        if (!Child)
        {
            continue;
        }

        if (bPromoteChildren)
        {
            Child->DestroyComponent(bPromoteChildren);
        }
        else
        {
            if (AttachParent)
            {
                // 자식 컴포넌트들을 부모에 어태치
                Child->SetupAttachment(AttachParent);
            }
            else if (GetOwner() && GetOwner()->GetRootComponent())
            {
                // 부모가 nullptr인 경우 Owner의 Root에라도 어태치
                Child->SetupAttachment(GetOwner()->GetRootComponent());
            }
            else if (GetOwner())
            {
                // 루트 컴포넌트도 없는 경우, 아무거나 하나를 루트로 지정해줌
                GetOwner()->SetRootComponent(Child);
            }
        }
    }

    AttachChildren.Empty();

    if (AttachParent)
    {
        DetachFromComponent(AttachParent);
    }

    UActorComponent::DestroyComponent(bPromoteChildren);
}

int USceneComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    int nIntersections = 0;
    return nIntersections;
}

FVector USceneComponent::GetForwardVector()
{
	FVector Forward = FVector(1.f, 0.f, 0.0f);
	Forward = JungleMath::FVectorRotate(Forward, QuatRotation);
	return Forward;
}

FVector USceneComponent::GetRightVector()
{
	FVector Right = FVector(0.f, 1.f, 0.0f);
	Right = JungleMath::FVectorRotate(Right, QuatRotation);
	return Right;
}

FVector USceneComponent::GetUpVector()
{
	FVector Up = FVector(0.f, 0.f, 1.0f);
	Up = JungleMath::FVectorRotate(Up, QuatRotation);
	return Up;
}


void USceneComponent::AddLocation(FVector _added)
{
	RelativeLocation = RelativeLocation + _added;

}

void USceneComponent::AddRotation(FVector _added)
{
	RelativeRotation = RelativeRotation + _added;

}

void USceneComponent::AddScale(FVector _added)
{
	RelativeScale3D = RelativeScale3D + _added;

}

void USceneComponent::UpdatePrevTransform()
{
    PrevRelativeLocation = RelativeLocation;
    PrevRelativeScale3D = RelativeScale3D;
    PrevQuatRotation = QuatRotation;
}

FVector USceneComponent::GetPrevWorldLocation()
{
    if (AttachParent)
    {
        return FVector(AttachParent->GetPrevWorldLocation() + GetPrevLocalLocation());
    }
    else
        return GetPrevLocalLocation();
}

FVector USceneComponent::GetPrevWorldRotation()
{
    if (AttachParent)
    {
        return FVector(AttachParent->GetPrevWorldRotation() + GetPrevLocalRotation());
    }
    else
        return GetPrevLocalRotation();
}

FVector USceneComponent::GetPrevWorldScale()
{
    if (AttachParent && dynamic_cast<USkySphereComponent*>(this))
    {
        return FVector(AttachParent->GetPrevWorldScale() + GetPrevLocalScale());
    }
    else
        return GetPrevLocalScale();
}

FVector USceneComponent::GetWorldRotation()
{
	if (AttachParent)
	{
		return FVector(AttachParent->GetWorldRotation() + GetLocalRotation());
	}
	else
		return GetLocalRotation();
}

FVector USceneComponent::GetWorldScale()
{
	if (AttachParent && dynamic_cast<USkySphereComponent*>(this))
	{
		return FVector(AttachParent->GetWorldScale() + GetLocalScale());
	}
	else
		return GetLocalScale();
}

FVector USceneComponent::GetWorldLocation()
{
	if (AttachParent)
	{
		return FVector(AttachParent->GetWorldLocation() + GetLocalLocation());
	}
	else
		return GetLocalLocation();
}

FVector USceneComponent::GetLocalRotation()
{
	return JungleMath::QuaternionToEuler(QuatRotation);
}

FVector USceneComponent::GetPrevLocalRotation()
{
    return JungleMath::QuaternionToEuler(PrevQuatRotation);
}

void USceneComponent::SetRotation(FVector _newRot)
{
	RelativeRotation = _newRot;
	QuatRotation = JungleMath::EulerToQuaternion(_newRot);
}

void USceneComponent::SetupAttachment(USceneComponent* InParent)
{
    // TODO: Attachment Rule 필요

    if (!InParent)
    {
        return;
    }

    USceneComponent* PrevParent = AttachParent;
    if (PrevParent && PrevParent != InParent)
    {
        DetachFromComponent(PrevParent);
    }

    AttachParent = InParent;
    InParent->AttachChildren.AddUnique(this);
}

void USceneComponent::DetachFromComponent(USceneComponent* Target)
{
    // TODO: Detachment Rule 필요

    if (!Target || !Target->AttachChildren.Contains(this))
    {
        return;
    }

    Target->AttachChildren.Remove(this);
}

void USceneComponent::DuplicateSubObjects()
{
    TArray<USceneComponent*> NewChildren = AttachChildren;
    AttachChildren.Empty();
    for (const auto& Child : NewChildren)
    {
        USceneComponent* NewChild = Cast<USceneComponent>(Child->Duplicate());
        NewChild->SetupAttachment(this);
        AttachChildren.Add(NewChild);
    }
}

UObject* USceneComponent::Duplicate()
{
    UObject* NewObject = FObjectFactory::ConstructObject<USceneComponent>(this);

    Cast<USceneComponent>(NewObject)->DuplicateSubObjects();
    return NewObject;
}


const TArray<USceneComponent*>& USceneComponent::GetAttachChildren() const
{
    return AttachChildren;
}

void USceneComponent::GetChildrenComponents(TArray<USceneComponent*>& Children) const
{
    Children.Empty();

    for (auto& Child : AttachChildren)
    {
        if (!Child)
        {
            // continue;
        }

        Children.Add(Child);

        TArray<USceneComponent*> ChildrenComp;
        Child->GetChildrenComponents(ChildrenComp);
        Children + ChildrenComp;
    }
}

USceneComponent* USceneComponent::GetAttachParent() const
{
    return AttachParent;
}

void USceneComponent::GetParentComponents(TArray<USceneComponent*>& Parents) const
{
    Parents.Empty();

    // �𸮾� �ҽ��ڵ� /Engine/Source/Runtime/Engine/Classes/Components/SceneComponent.h ����
    USceneComponent* ParentIterator = GetAttachParent();
    while (ParentIterator != nullptr)
    {
        Parents.Add(ParentIterator);
        ParentIterator = ParentIterator->GetAttachParent();
    }
}
