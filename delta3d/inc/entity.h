// entity.h: Declaration of the Entity class.
//
//////////////////////////////////////////////////////////////////////

#ifndef DT_HLA_ENTITY
#define DT_HLA_ENTITY

#include <string>

#include "dis_types.h"
#include "object.h"


namespace dtHLA
{
   /**
    * A DIS/RPR-FOM entity.
    */
   class Entity : public dtCore::Object
   {
      DECLARE_MANAGEMENT_LAYER(Entity)


      public:

         /**
          * Constructor.
          *
          * @param name the instance name
          */
         Entity(std::string name = "Entity");

         /**
          * Destructor.
          */
         virtual ~Entity();

         /**
          * Sets this entity's DIS/RPR-FOM entity identifier.
          *
          * @param entityIdentifier the entity identifier to copy
          */
         void SetEntityIdentifier(const EntityIdentifier& entityIdentifier);

         /**
          * Retrieves this entity's DIS/RPR-FOM entity identifier.
          *
          * @return the entity identitifier
          */
         const EntityIdentifier& GetEntityIdentifier();

         /**
          * Sets this entity's DIS/RPR-FOM entity type.
          *
          * @param entityType the entity type to copy
          */
         void SetEntityType(const EntityType& entityType);

         /**
          * Retrieves this entity's DIS/RPR-FOM entity type.
          *
          * @return the entity type
          */
         const EntityType& GetEntityType();
         
         /**
          * Sets this entity's DIS/RPR-FOM world location.
          *
          * @param worldLocation the world location to copy
          */
         void SetWorldLocation(const WorldCoordinate& worldLocation);

         /**
          * Retrieves this entity's DIS/RPR-FOM world location.
          *
          * @return the world location
          */
         const WorldCoordinate& GetWorldLocation();
         
         /**
          * Sets this entity's DIS/RPR-FOM orientation.
          *
          * @param orientation the orientation to copy
          */
         void SetOrientation(const EulerAngles& orientation);

         /**
          * Retrieves this entity's DIS/RPR-FOM orientation.
          *
          * @return the orientation
          */
         const EulerAngles& GetOrientation();
         
         /**
          * Sets this entity's DIS/RPR-FOM velocity vector.
          *
          * @param velocityVector the velocity vector to copy
          */
         void SetVelocityVector(const VelocityVector& velocityVector);

         /**
          * Retrieves this entity's DIS/RPR-FOM velocity vector.
          *
          * @return the velocity vector
          */
         const VelocityVector& GetVelocityVector();
         
         /**
          * Sets this entity's DIS/RPR-FOM acceleration vector.
          *
          * @param accelerationVector the acceleration vector to copy
          */
         void SetAccelerationVector(const VelocityVector& accelerationVector);

         /**
          * Retrieves this entity's DIS/RPR-FOM acceleration vector.
          *
          * @return the acceleration vector
          */
         const VelocityVector& GetAccelerationVector();
         
         /**
          * Sets this entity's DIS/RPR-FOM angular velocity vector.
          *
          * @param angularVelocityVector the angular velocity vector to copy
          */
         void SetAngularVelocityVector(const VelocityVector& angularVelocityVector);

         /**
          * Retrieves this entity's DIS/RPR-FOM angular velocity vector.
          *
          * @return the angular velocity vector
          */
         const VelocityVector& GetAngularVelocityVector();


      private:

         /**
          * The entity's DIS/RPR-FOM entity identifier.
          */
         EntityIdentifier mEntityIdentifier;

         /**
          * The entity's DIS/RPR-FOM entity type.
          */
         EntityType mEntityType;
         
         /**
          * The entity's DIS/RPR-FOM world location.
          */
         WorldCoordinate mWorldLocation;
         
         /**
          * The entity's DIS/RPR-FOM orientation.
          */
         EulerAngles mOrientation;
         
         /**
          * The entity's DIS/RPR-FOM velocity vector.
          */
         VelocityVector mVelocityVector;
         
         /**
          * The entity's DIS/RPR-FOM acceleration vector.
          */
         VelocityVector mAccelerationVector;
         
         /**
          * The entity's DIS/RPR-FOM angular velocity vector.
          */
         VelocityVector mAngularVelocityVector;
   };
};

#endif // DT_HLA_ENTITY
