// Copyright Epic Games, Inc. All Rights Reserved.

#include "MassECS/MassEntityTestCombatTrait.h"
#include "MassEntityTemplateRegistry.h"
#include "MassCommonFragments.h"
#include "MassEntityUtils.h"
#include "MassEntityManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(MassEntityTestCombatTrait)

void UMassEntityTestCombatTrait::BuildTemplate(FMassEntityTemplateBuildContext& BuildContext, const UWorld& World) const
{
	FMassEntityManager& EntityManager = UE::Mass::Utils::GetEntityManagerChecked(World);

	BuildContext.AddFragment<FTransformFragment>();
	BuildContext.AddFragment(FConstStructView::Make(Health));
	BuildContext.AddFragment(FConstStructView::Make(Combat));
	BuildContext.AddFragment(FConstStructView::Make(Team));
	BuildContext.AddFragment(FConstStructView::Make(Movement));
	BuildContext.AddFragment<FMassEntityTestTargetFragment>();

	const FConstSharedStruct VisualFragment = EntityManager.GetOrCreateConstSharedFragment(Visual);
	BuildContext.AddConstSharedFragment(VisualFragment);
}
