# Freedom Phantoms

<a href='https://rareorb.itch.io/freedomphantoms'><img width="150"   alt='Get it on itch.io' src='https://static.itch.io/images/badge-color.svg'/></a>


## Watch Gameplay Trailer
[![Watch Gameplay Trailer](https://vumbnail.com/902035926.jpg)](https://vimeo.com/902035926)


## About
Freedom Phantoms is a Third-person shooter inspired by Call of Duty: Modern Warfare 2 (2009 & 2020) & Freedom Fighters. With the fusion of audio, 3D models and other resources extracted from Modern Warfare 2 as well as gameplay mechanics from Freedom Fighters, the end result would simulate a similar gameplay experience which I personally enjoyed when playing with these products.

The purpose of creating this game was to gain exposure to Unreal Engine tools as much as possible. As a developer, I looked towards improving my technical & interpersonal skills when developing this project. Given my focus as a programmer, I heavily relied on third party resources such as 3D models, character animations etc. as my skillset outside of a coder is very little. Since my plan was not to make any commercial profit from this personal project, I decided it would not be in my best interest to buy these reources as even buying an character animation or character model pack would be quite expensive. This then drew my attention to extract Modern Warfare 2 models as the resources such as artwork seemed quite impressive.

As mentioned, this is a personal project not used for commerical purposes.


## Design Patterns

* Object Pooling - Use mainly in the projectile instantiation to help maintain performance by recycling actors.

## AI System

* Utility AI - Using a score system to set the NPC behaviour rather than using the built-in Unreal's Behaviour Tree. Using C++ was a lot of easier to develop the system as it followed the use of an Actor Component and an empty Unreal abstract class. Code was built from another user's codebase, see references.

## References

CALL OF DUTY: MODERN WARFARE 2 CAMPAIGN REMASTERED - https://www.callofduty.com/mw2campaignremastered

Freedom Fighters - https://en.wikipedia.org/wiki/Freedom_Fighters_(video_game)

Utility AI - https://github.com/rdeioris/UnrealUtilityAI